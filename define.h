#pragma  once



#include "stdafx.h"
#include <string.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>

using namespace std;


#define WSA_RECV         0
#define WSA_SEND         1

#define DATA_BUFSIZE    8192
#define MAX_NAME_LEN    128
#define MAX_PWD_LEN     128
#define MAX_RESP_LEN    1024
#define MAX_REQ_LEN     256
#define MAX_ADDR_LEN    80

#define FTP_PORT        21     // FTP 控制端口
#define DATA_FTP_PORT   20     // FTP 数据端口

#define USER_OK         331
#define LOGGED_IN       230
#define LOGIN_FAILED    530
#define CMD_OK          200
#define OPENING_AMODE   150
#define TRANS_COMPLETE  226
#define CANNOT_FIND     550
#define FTP_QUIT        221
#define CURR_DIR        257
#define DIR_CHANGED     250
#define OS_TYPE         215
#define REPLY_MARKER    504
#define PASSIVE_MODE    227

#define FTP_USER		"toldo"
#define FTP_PASS		"toldo"
#define DEFAULT_HOME_DIR    "C:\\temp"

#define MAX_FILE_NUM        1024

#define MODE_PORT       0
#define MODE_PASV       1

#define PORT_BIND   1821





typedef struct {
	CHAR   buffRecv[DATA_BUFSIZE];
	CHAR   buffSend[DATA_BUFSIZE];
	WSABUF wsaBuf;
	SOCKET s;
	WSAOVERLAPPED o;
	DWORD dwBytesSend;
	DWORD dwBytesRecv;
	int   nStatus;
} SOCKET_INF, *LPSOCKET_INF;

typedef struct {
	TCHAR    szFileName[MAX_PATH];
	DWORD    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD    nFileSizeHigh;
	DWORD    nFileSizeLow;
} FILE_INF, *LPFILE_INF;



/************************/
//描述：char转为tchar
/************************/
static void CharToTChar(const char* pszSource, TCHAR* pszDest)
{
	if (NULL != pszSource && NULL != pszDest)
	{
		int nLength = MultiByteToWideChar(CP_ACP, 0, pszSource, strlen(pszSource) + 1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, pszSource, strlen(pszSource) + 1, (LPWSTR)pszDest, nLength);
	}
}

/************************/
//描述：tchar转为char
/************************/
static void TCharToChar(const TCHAR* pszSource, char* pszDest)
{
	if (NULL != pszSource && NULL != pszDest)
	{
		int nLength = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pszSource, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pszSource, -1, pszDest, nLength, NULL, NULL);
	}
}

/************************/
//描述：char转为tchar
/************************/
static TCHAR*  CharToTChar(const char* pszSource)
{
	TCHAR* pszDest =new TCHAR[MAX_PATH];
	if (NULL != pszSource && NULL != pszDest)
	{
		int nLength = MultiByteToWideChar(CP_ACP, 0, pszSource, strlen(pszSource) + 1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, pszSource, strlen(pszSource) + 1,(LPWSTR) pszDest, nLength);
	
	}
	return pszDest;
}

/************************/
//描述：tchar转为char
/************************/
static  char*  TCharToChar(const TCHAR* pszSource)
{
	char* pszDest=new char[MAX_PATH];
	if (NULL != pszSource && NULL != pszDest)
	{
		int nLength = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pszSource, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pszSource, -1, pszDest, nLength, NULL, NULL);
	}
	return pszDest;
}


///////////////////////////////////////////////////////////////////////////////////////////
//其他函数
static char* GetLocalAddress()
{
	struct in_addr *pinAddr;
	LPHOSTENT	lpHostEnt;
	int			nRet;
	int			nLen;
	char        szLocalAddr[80];
	memset(szLocalAddr, 0, sizeof(szLocalAddr));
	// Get our local name
	nRet = gethostname(szLocalAddr, sizeof(szLocalAddr));
	if (nRet == SOCKET_ERROR)
	{
		return NULL;
	}
	// "Lookup" the local name
	lpHostEnt = gethostbyname(szLocalAddr);
	if (NULL == lpHostEnt)
	{
		return NULL;
	}
	// Format first address in the list
	pinAddr = ((LPIN_ADDR)lpHostEnt->h_addr);
	nLen = strlen(inet_ntoa(*pinAddr));
	if ((DWORD)nLen > sizeof(szLocalAddr))
	{
		WSASetLastError(WSAEINVAL);
		return NULL;
	}
	return inet_ntoa(*pinAddr);
}

static int GetFileList(LPFILE_INF pFI, UINT nArraySize, const char* szPath)
{
	WIN32_FIND_DATA  wfd;
	int idx = 0;
	CHAR lpFileName[MAX_PATH];
#ifdef UNICODE
	TCHAR  tszCurrDir[MAX_PATH];
	TCHAR  tszPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, tszCurrDir);
	CharToTChar(szPath,tszPath);
	_tcscat(tszCurrDir, L"\\");
	_tcscat(tszCurrDir, tszPath);
	HANDLE hFile = FindFirstFile(tszCurrDir, &wfd);
#else
	GetCurrentDirectory(MAX_PATH, lpFileName);
	strcat(lpFileName, "\\");
	strcat(lpFileName, szPath);
	HANDLE hFile = FindFirstFile(lpFileName, &wfd);
#endif	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		pFI[idx].dwFileAttributes = wfd.dwFileAttributes;
		lstrcpy(pFI[idx].szFileName, wfd.cFileName);
		pFI[idx].ftCreationTime = wfd.ftCreationTime;
		pFI[idx].ftLastAccessTime = wfd.ftLastAccessTime;
		pFI[idx].ftLastWriteTime = wfd.ftLastWriteTime;
		pFI[idx].nFileSizeHigh = wfd.nFileSizeHigh;
		pFI[idx++].nFileSizeLow = wfd.nFileSizeLow;
		while (FindNextFile(hFile, &wfd) && idx < (int)nArraySize)
		{
			pFI[idx].dwFileAttributes = wfd.dwFileAttributes;
			lstrcpy(pFI[idx].szFileName, wfd.cFileName);
			pFI[idx].ftCreationTime = wfd.ftCreationTime;
			pFI[idx].ftLastAccessTime = wfd.ftLastAccessTime;
			pFI[idx].ftLastWriteTime = wfd.ftLastWriteTime;
			pFI[idx].nFileSizeHigh = wfd.nFileSizeHigh;
			pFI[idx++].nFileSizeLow = wfd.nFileSizeLow;
		}
		FindClose(hFile);
	}
	return idx;
}
static char* HostToNet(char* szPath)
{
	int idx = 0;
	if (NULL == szPath) return NULL;
	strlwr(szPath);
	while (szPath[idx])
	{
		if (szPath[idx] == '\\')
			szPath[idx] = '/';
		idx++;
	}
	return szPath;
}

static char* NetToHost(char* szPath)
{
	int idx = 0;
	if (NULL == szPath) return NULL;
	strlwr(szPath);
	while (szPath[idx])
	{
		if ('/' == szPath[idx])
			szPath[idx] = '\\';
		idx++;
	}
	return szPath;
}
static char* RelativeDirectory(char* szDir)
{
	int nStrLen = strlen(DEFAULT_HOME_DIR);
	if (!strnicmp(szDir, DEFAULT_HOME_DIR, nStrLen))
		szDir += nStrLen;

	if (szDir && szDir[0] == '\0') szDir = "/";

	return HostToNet(szDir);
}
static char* AbsoluteDirectory(char* szDir)
{
	char szTemp[MAX_PATH];
	strcpy(szTemp, DEFAULT_HOME_DIR + 2);
	if (NULL == szDir) return NULL;
	if ('/' == szDir[0])
		strcat(szTemp, szDir);
	szDir = szTemp;
	return NetToHost(szDir);
}



static std::string string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}
//////////////////////////////////////////////////////////////////////////
static std::string UTF8_To_string(const std::string & str)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;

	return retStr;
}
//////////////////////////////////////////////////////////////////////////


//////////



