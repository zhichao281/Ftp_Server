#pragma  once

#include "define.h"

class CProtocol
{

public:
	CProtocol();
	virtual ~CProtocol();


	DWORD Run();

	HANDLE Do();

	//工作者线程处理函数
	static void ProcessTreadIO(void *lpParameter);

	BOOL WelcomeInfo(SOCKET s);

	int LoginIn(LPSOCKET_INF pSocketInfo);

	int SendRes(LPSOCKET_INF pSI);

	int RecvReq(LPSOCKET_INF pSI);

	int DealCommand(LPSOCKET_INF pSI);

	SOCKET DataAccept(SOCKET& s);

	char* ConvertCommaAddress(char* szAddress, WORD wPort);

	int ConvertDotAddress(char* szAddress, LPDWORD pdwIpAddr, LPWORD pwPort);

	UINT FileListToString(char* buff, UINT nBuffSize, BOOL bDetails);

	DWORD ReadFileToBuffer(const char* szFile, char *buff, DWORD nFileSize);

	DWORD WriteToFile(SOCKET s, const char* szFile);

	int CombindFileNameSize(const char* szFileName, char* szFileNS);

	int	DataConn(SOCKET& s, DWORD dwIp, WORD wPort, int nMode);

	int DataSend(SOCKET s, char* buff, int nBufSize);

	int DataRecv(SOCKET s, const char* szFileName);


private:
	DWORD m_index;
	BOOL  m_bLoggedIn;
	BOOL m_bRun;
};

