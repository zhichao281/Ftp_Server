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
	/** 单例模式获取类指针函数，使用此模式进行获取 */
	static CFtpApp* GetInstance();
public:
	map<string ,SInfo> m_map;
	static string GetString(LPCSTR s,LPSTR sKey = NULL);

	//查询是否是已经开始通讯
	BOOL Allow();

	BOOL Start();

	BOOL InitInstance();
	
	// 异步开启
	bool	AsynStart();

private:
	static CFtpApp* 			g_spFtpAppPlug ;   /** 单例模式自身静态变量 */

public:

	DWORD g_dwEventTotal ;	
	WSAEVENT g_events[WSA_MAXIMUM_WAIT_EVENTS];
	LPSOCKET_INF g_sockets[WSA_MAXIMUM_WAIT_EVENTS];

	SOCKET           m_sListen;
	CRITICAL_SECTION g_cs;//异步线程锁

	char  m_szLocalAddr[MAX_ADDR_LEN];  //本机地址
	
	std::thread*			m_pThread;
};


/** 单例模式类调用宏定义，方便调用 */
#define  CFtpAppSingleton  CFtpApp::GetInstance()

#endif // !defined(AFX_FTPAPP_H__0267EDD2_9DB8_4968_8FB0_0CFD91B9B74C__INCLUDED_)
