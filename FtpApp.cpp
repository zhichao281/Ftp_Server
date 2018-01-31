// FtpApp.cpp: implementation of the CFtpApp class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "FtpApp.h"
#include "Protocol.h"

CFtpApp* CFtpApp::g_spFtpAppPlug = nullptr;         /** ��ʼ��Ϊ��ָ�� */



CFtpApp::CFtpApp()
{
	g_dwEventTotal = 0;
	m_pThread = nullptr;
}

CFtpApp::~CFtpApp()
{

}

CFtpApp* CFtpApp::GetInstance()
{
	if (nullptr == g_spFtpAppPlug)
	{
		g_spFtpAppPlug = new CFtpApp;
	}
	return g_spFtpAppPlug;
}
string CFtpApp::GetString(LPCSTR s,LPSTR sKey)
{
	return "";
}	
BOOL CFtpApp::InitInstance()
{
	return TRUE;
}
BOOL CFtpApp::Allow()
{
	return TRUE;
}
// �첽����
bool CFtpApp::AsynStart()
{

	if (m_pThread == NULL)
	{
		m_pThread = new std::thread(std::bind(&CFtpApp::Start, this));
	}
	return true;
}
BOOL CFtpApp::Start()
{
	WSADATA wsaData;
	SOCKET sListen, sAccept;
	SOCKADDR_IN inetAddr;
	DWORD dwFlags;
	DWORD dwThreadId;
	DWORD dwRecvBytes;
	INT   nRet;
	printf("Mini Ftpserver�Ѿ����� \n");
	InitializeCriticalSection(&g_cs);
	if ((nRet = WSAStartup(0x0202, &wsaData)) != 0) {
		printf("����WSAStartup failed with error %d\n", nRet);
		return false;
	}

	// ��ȡ�ñ��ص�ַ
	sprintf(m_szLocalAddr, "%s", GetLocalAddress());
	if ((sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("����Failed to get a socket %d\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	inetAddr.sin_family = AF_INET;
	inetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inetAddr.sin_port = htons(FTP_PORT);

	if (::bind(sListen, (PSOCKADDR)&inetAddr, sizeof(inetAddr)) == SOCKET_ERROR)
	{
		printf("����bind() failed with error %d\n", WSAGetLastError());
		return false;
	}

	if (listen(sListen, SOMAXCONN))
	{
		printf("����listen() failed with error %d\n", WSAGetLastError());
		return false;
	}

	printf("Mini Ftpserver��ʼ���� \n");

	if ((sAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("����Failed to get a socket %d\n", WSAGetLastError());
		return false;
	}

	//������һ���ֶ����ö��� 
	if ((g_events[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("����WSACreateEvent failed with error %d\n", WSAGetLastError());
		return false;
	}


	CProtocol *pProto = new CProtocol;

	// ����һ���̴߳�������
	if (pProto->Do() == NULL)
	{
		printf("����CreateThread failed with error %d\n", GetLastError());
		return false;
	}

	g_dwEventTotal = 1;

	while (TRUE)
	{
		//������վ����
		if ((sAccept = accept(sListen, NULL, NULL)) == INVALID_SOCKET)
		{
			printf("����accept failed with error %d\n", WSAGetLastError());
			return false;
		}

		//�ش���ӭ��Ϣ
		if (!pProto->WelcomeInfo(sAccept)) break;
		//����ftp��Ŀ¼
#ifdef UNICODE
		if (!SetCurrentDirectory(CharToTChar(DEFAULT_HOME_DIR))) break;
#else
		if (!SetCurrentDirectory(DEFAULT_HOME_DIR)) break;
#endif


		//�����ٽ�������ֹ����
		EnterCriticalSection(&g_cs);
		//����һ���µ�SOCKET_INF�ṹ������ܵ�����socket.
		if ((g_sockets[g_dwEventTotal] = (LPSOCKET_INF)
			GlobalAlloc(GPTR, sizeof(SOCKET_INF))) == NULL)
		{
			printf("����GlobalAlloc() failed with error %d\n", GetLastError());
			return false;
		}

		//��ʼ���µ�SOCKET_INF�ṹ
		char buff[DATA_BUFSIZE]; memset(buff, 0, DATA_BUFSIZE);
		g_sockets[g_dwEventTotal]->wsaBuf.buf = buff;
		g_sockets[g_dwEventTotal]->wsaBuf.len = DATA_BUFSIZE;
		g_sockets[g_dwEventTotal]->s = sAccept;
		memset(&(g_sockets[g_dwEventTotal]->o), 0, sizeof(OVERLAPPED));
		g_sockets[g_dwEventTotal]->dwBytesSend = 0;
		g_sockets[g_dwEventTotal]->dwBytesRecv = 0;
		g_sockets[g_dwEventTotal]->nStatus = WSA_RECV;    // ����

		//�����¼�
		if ((g_sockets[g_dwEventTotal]->o.hEvent = g_events[g_dwEventTotal] =
			WSACreateEvent()) == WSA_INVALID_EVENT)
		{
			printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
			return false;
		}

		//������������
		dwFlags = 0;
		if (WSARecv(g_sockets[g_dwEventTotal]->s,
			&(g_sockets[g_dwEventTotal]->wsaBuf), 1, &dwRecvBytes, &dwFlags,
			&(g_sockets[g_dwEventTotal]->o), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("����WSARecv() failed with error %d\n", WSAGetLastError());
				return false;
			}
		}
		g_dwEventTotal++;

		//�뿪�ٽ���
		LeaveCriticalSection(&g_cs);

		//ʹ��һ���¼����źš�ʹ�������̴߳����������¼�
		if (WSASetEvent(g_events[0]) == FALSE)
		{
			printf("����WSASetEvent failed with error %d\n", WSAGetLastError());
			return false;
		}
	}
	return true;
}
