// FtpApp.h: interface for the CFtpApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FTPAPP_H__0267EDD2_9DB8_4968_8FB0_0CFD91B9B74C__INCLUDED_)
#define AFX_FTPAPP_H__0267EDD2_9DB8_4968_8FB0_0CFD91B9B74C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "define.h"

#include <map>
#include <thread>

struct SInfo
{
	string szPass;
	string szPath;
};

class CFtpApp 
{
private:
	CFtpApp();
	virtual ~CFtpApp();
public:
	/** ����ģʽ��ȡ��ָ�뺯����ʹ�ô�ģʽ���л�ȡ */
	static CFtpApp* GetInstance();
public:
	map<string ,SInfo> m_map;
	static string GetString(LPCSTR s,LPSTR sKey = NULL);

	//��ѯ�Ƿ����Ѿ���ʼͨѶ
	BOOL Allow();

	BOOL Start();

	BOOL InitInstance();
	
	// �첽����
	bool	AsynStart();

private:
	static CFtpApp* 			g_spFtpAppPlug ;   /** ����ģʽ����̬���� */

public:

	DWORD g_dwEventTotal ;	
	WSAEVENT g_events[WSA_MAXIMUM_WAIT_EVENTS];
	LPSOCKET_INF g_sockets[WSA_MAXIMUM_WAIT_EVENTS];

	SOCKET           m_sListen;
	CRITICAL_SECTION g_cs;//�첽�߳���

	char  m_szLocalAddr[MAX_ADDR_LEN];  //������ַ
	
	std::thread*			m_pThread;
};


/** ����ģʽ����ú궨�壬������� */
#define  CFtpAppSingleton  CFtpApp::GetInstance()

#endif // !defined(AFX_FTPAPP_H__0267EDD2_9DB8_4968_8FB0_0CFD91B9B74C__INCLUDED_)
