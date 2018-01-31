// FtpApp.cpp: implementation of the CFtpApp class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "FtpApp.h"
#include "Protocol.h"

CFtpApp* CFtpApp::g_spFtpAppPlug = nullptr;         /** 初始化为空指针 */



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
// 异步开启
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
	printf("Mini Ftpserver已经启动 \n");
	InitializeCriticalSection(&g_cs);
	if ((nRet = WSAStartup(0x0202, &wsaData)) != 0) {
		printf("错误：WSAStartup failed with error %d\n", nRet);
		return false;
	}

	// 先取得本地地址
	sprintf(m_szLocalAddr, "%s", GetLocalAddress());
	if ((sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("错误：Failed to get a socket %d\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	inetAddr.sin_family = AF_INET;
	inetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inetAddr.sin_port = htons(FTP_PORT);

	if (::bind(sListen, (PSOCKADDR)&inetAddr, sizeof(inetAddr)) == SOCKET_ERROR)
	{
		printf("错误：bind() failed with error %d\n", WSAGetLastError());
		return false;
	}

	if (listen(sListen, SOMAXCONN))
	{
		printf("错误：listen() failed with error %d\n", WSAGetLastError());
		return false;
	}

	printf("Mini Ftpserver开始侦听 \n");

	if ((sAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("错误：Failed to get a socket %d\n", WSAGetLastError());
		return false;
	}

	//创建第一个手动重置对象 
	if ((g_events[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("错误：WSACreateEvent failed with error %d\n", WSAGetLastError());
		return false;
	}


	CProtocol *pProto = new CProtocol;

	// 创建一个线程处理请求
	if (pProto->Do() == NULL)
	{
		printf("错误：CreateThread failed with error %d\n", GetLastError());
		return false;
	}

	g_dwEventTotal = 1;

	while (TRUE)
	{
		//处理入站连接
		if ((sAccept = accept(sListen, NULL, NULL)) == INVALID_SOCKET)
		{
			printf("错误：accept failed with error %d\n", WSAGetLastError());
			return false;
		}

		//回传欢迎消息
		if (!pProto->WelcomeInfo(sAccept)) break;
		//设置ftp根目录
#ifdef UNICODE
		if (!SetCurrentDirectory(CharToTChar(DEFAULT_HOME_DIR))) break;
#else
		if (!SetCurrentDirectory(DEFAULT_HOME_DIR)) break;
#endif


		//操作临界区，防止出错
		EnterCriticalSection(&g_cs);
		//创建一个新的SOCKET_INF结构处理接受的数据socket.
		if ((g_sockets[g_dwEventTotal] = (LPSOCKET_INF)
			GlobalAlloc(GPTR, sizeof(SOCKET_INF))) == NULL)
		{
			printf("错误：GlobalAlloc() failed with error %d\n", GetLastError());
			return false;
		}

		//初始化新的SOCKET_INF结构
		char buff[DATA_BUFSIZE]; memset(buff, 0, DATA_BUFSIZE);
		g_sockets[g_dwEventTotal]->wsaBuf.buf = buff;
		g_sockets[g_dwEventTotal]->wsaBuf.len = DATA_BUFSIZE;
		g_sockets[g_dwEventTotal]->s = sAccept;
		memset(&(g_sockets[g_dwEventTotal]->o), 0, sizeof(OVERLAPPED));
		g_sockets[g_dwEventTotal]->dwBytesSend = 0;
		g_sockets[g_dwEventTotal]->dwBytesRecv = 0;
		g_sockets[g_dwEventTotal]->nStatus = WSA_RECV;    // 接收

		//创建事件
		if ((g_sockets[g_dwEventTotal]->o.hEvent = g_events[g_dwEventTotal] =
			WSACreateEvent()) == WSA_INVALID_EVENT)
		{
			printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
			return false;
		}

		//发出接受请求
		dwFlags = 0;
		if (WSARecv(g_sockets[g_dwEventTotal]->s,
			&(g_sockets[g_dwEventTotal]->wsaBuf), 1, &dwRecvBytes, &dwFlags,
			&(g_sockets[g_dwEventTotal]->o), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				printf("错误：WSARecv() failed with error %d\n", WSAGetLastError());
				return false;
			}
		}
		g_dwEventTotal++;

		//离开临界区
		LeaveCriticalSection(&g_cs);

		//使第一个事件有信号。使工作者线程处理其他的事件
		if (WSASetEvent(g_events[0]) == FALSE)
		{
			printf("错误：WSASetEvent failed with error %d\n", WSAGetLastError());
			return false;
		}
	}
	return true;
}
