// +----------------------------------------------------------------------
// | Green C++ Basis Components
// +----------------------------------------------------------------------
// | GSThreadLock	线程锁
// | GSThreadPool	线程池
// +----------------------------------------------------------------------
// | Author:	Maga
// +----------------------------------------------------------------------
// | Modify:	Maga
// +----------------------------------------------------------------------


#include "stdafx.h"
#include "GSThreadPool.h"


/*
 *	Function:		GSThreadLock
 *	Explanation:	构造函数
 */
GSThreadLock::GSThreadLock()
{
	InitializeCriticalSection(&m_Section);
}

/*
*	Function:		~GSThreadLock
*	Explanation:	析构函数
*/
GSThreadLock::~GSThreadLock()
{
	DeleteCriticalSection(&m_Section);
}

/*
 *	Function:		Lock
 *	Explanation:	锁
 */
void GSThreadLock::Lock()
{
	EnterCriticalSection(&m_Section);
}

/*
*	Function:		UnLock
*	Explanation:	解锁
*/
void GSThreadLock::UnLock()
{
	LeaveCriticalSection(&m_Section);
}



/*
 *	Function:		GSThreadPool
 *	Explanation:	构造函数
 */
GSThreadPool::GSThreadPool()
{
	_Init();
	Start();
}

/*
 *	Function:		~GSThreadPool
 *	Explanation:	析构函数
 */
GSThreadPool::~GSThreadPool()
{
	m_bStop = true;
	_ClearQueue();
	_CloseThreads();
}

/*
 *	Function:		_Init
 *	Explanation:	初始化
 */
void GSThreadPool::_Init()
{
	m_bStop = true;
	m_QueueList.clear();

	for (int i = 0; i < DEFAULT_THREAD_COUNT; i++)
	{
		m_hThread[i] = NULL;
	}
}

/*
 *	Function:		_ClearQueue
 *	Explanation:	清空队列
 */
void GSThreadPool::_ClearQueue()
{
	m_Lock.Lock();

	for (size_t i = 0; i < m_QueueList.size(); i++)
	{
		POOL_DATA			PoolData			= { 0 };

		PoolData = m_QueueList.front();

		if (PoolData.bAutoDel)
		{
			delete PoolData.pThreadData;
		}

		m_QueueList.pop_front();
	}

	m_QueueList.clear();
	m_Lock.UnLock();
}

/*
 *	Function:		_CloseThreads
 *	Explanation:	关闭线程
 */
void GSThreadPool::_CloseThreads()
{
	for (int i = 0; i < DEFAULT_THREAD_COUNT; i++)
	{
		if (NULL != m_hThread[i])
		{
			if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread[i], 10))
			{
				TerminateThread(m_hThread[i], 0);
			}
			
			CloseHandle(m_hThread[i]);
			m_hThread[i] = NULL;
		}
	}
}

/*
 *	Function:		Start
 *	Explanation:	启动线程池
 *	Return:			true 成功 false 失败
 */
bool GSThreadPool::Start()
{
	if (!m_bStop)
	{
		return true;
	}

	m_bStop = false;

	for (int i = 0; i < DEFAULT_THREAD_COUNT; i++)
	{
		m_hThread[i] = CreateThread(NULL, 0, WorkingThread, this, 0, NULL);
	}

	return true;
}

/*
 *	Function:		Stop
 *	Explanation:	停止
 */
void GSThreadPool::Stop()
{
	m_bStop = true;
	_ClearQueue();
	_CloseThreads();
}

/*
 *	Function:		IsRunning
 *	Explanation:	线程池是否在运行
 *	Return:			true 正在运行 false 停止
 */
bool GSThreadPool::IsRunning()
{
	return !m_bStop;
}

/*
 *	Function:		AddWork
 *	Explanation:	添加工作到队列
 *	Input:			Fun			函数
					pData		数据
					bPriority	是否要优先执行
					bAutoDel	是否需要自动delete掉pData
 *	Return:			true 成功 false 失败
 */
bool GSThreadPool::AddWork(std::function<void(void *)> Fun, void *pData, bool bPriority /* = false */, bool bAutoDel /* = false */)
{
	POOL_DATA			PoolData				= { 0 };


	if (m_bStop)
	{
		return false;
	}

	PoolData.bAutoDel = bAutoDel;
	PoolData.ThreadFun = Fun;
	PoolData.pThreadData = pData;

	return Push(PoolData, bPriority);
}

/*
 *	Function:		ExcNow
 *	Explanation:	马上执行
 *	Input:			Fun			函数
					pData		数据
					bAutoDel	是否需要自动delete掉pData
 *	Return:			true 成功 false 失败
 */
bool GSThreadPool::ExcNow(std::function<void(void *)> Fun, void *pData, bool bAutoDel /* = false */)
{
	POOL_DATA*			pPoolData				= new POOL_DATA;
	HANDLE				hThread					= NULL;

	pPoolData->ThreadFun = Fun;
	pPoolData->pThreadData = pData;
	pPoolData->bAutoDel = bAutoDel;
	hThread = CreateThread(NULL, 0, OneWorkThread, pPoolData, 0, NULL);

	if (NULL == hThread)
	{
		delete pPoolData;
		return false;
	}

	CloseHandle(hThread);

	return true;
}

/*
*	Function:		ExcNow2
*	Explanation:	马上执行
*	Input:			Fun			函数
					pData		数据
					bAutoDel	是否需要自动delete掉pData
*	Return:			句柄
*/
HANDLE GSThreadPool::ExcNow2(std::function<void(void *)> Fun, void *pData, bool bAutoDel /* = false */)
{
	POOL_DATA*			pPoolData				= new POOL_DATA;
	HANDLE				hThread					= NULL;

	pPoolData->ThreadFun = Fun;
	pPoolData->pThreadData = pData;
	pPoolData->bAutoDel = bAutoDel;
	hThread = CreateThread(NULL, 0, OneWorkThread, pPoolData, 0, NULL);

	if (NULL == hThread)
	{
		delete pPoolData;
		return NULL;
	}

	return hThread;
}
/*
 *	Function:		Pop
 *	Explanation:	从队列取出线程数据
 *	Input:			PoolData		线程数据
 *	Return:			true 成功 false 失败
 */
bool GSThreadPool::Pop(POOL_DATA &PoolData)
{
	m_Lock.Lock();

	if (0 >= m_QueueList.size())
	{
		// 当前工作队列为空
		m_Lock.UnLock();
		return false;
	}

	PoolData = m_QueueList.front();
	m_QueueList.pop_front();
	m_Lock.UnLock();

	return true;
}

/*
 *	Function:		Push
 *	Explanation:	添加线程数据到队列中
 *	Input:			PoolData		线程数据
					bPriority		是否优先执行
 *	Return:			true 成功 false 失败
 */
bool GSThreadPool::Push(POOL_DATA PoolData, bool bPriority /* = false */)
{
	m_Lock.Lock();

	if (bPriority)
	{
		m_QueueList.push_front(PoolData);
	}
	else
	{
		m_QueueList.push_back(PoolData);
	}

	m_Lock.UnLock();
	return true;
}

/*
 *	Function:		Working
 *	Explanation:	工作线程主体
 */
void GSThreadPool::Working()
{


	for (; !m_bStop; Sleep(10))
	{
		POOL_DATA			PoolData				= { 0 };

		if (false == Pop(PoolData))
		{
			continue;
		}

		PoolData.ThreadFun(PoolData.pThreadData);

		if (PoolData.bAutoDel)
		{
			delete PoolData.pThreadData;
		}
	}
}

/*
 *	Function:		WorkingThread
 *	Explanation:	工作线程
 *	Input:			pParam		线程参数
 *	Return:			S_OK 成功 S_FALSE 失败
 */
unsigned long CALLBACK GSThreadPool::WorkingThread(void *pParam)
{
	GSThreadPool*			pThis				= (GSThreadPool *)pParam;

	if (NULL == pThis)
	{
		return S_FALSE;
	}

	pThis->Working();

	return S_OK;
}

/*
 *	Function:		OneWorkThread
 *	Explanation:	单个工作线程
 *	Input:			pParam		线程参数
 *	Return:			S_OK 成功 S_FALSE 失败
 */
unsigned long CALLBACK GSThreadPool::OneWorkThread(void *pParam)
{
	POOL_DATA*				pPoolData			= (POOL_DATA *)pParam;

	if (NULL == pPoolData)
	{
		return S_FALSE;
	}

	pPoolData->ThreadFun(pPoolData->pThreadData);

	if (pPoolData->bAutoDel)
	{
		delete pPoolData->pThreadData;
	}

	delete pPoolData;
	return S_OK;
}

/*
 *	Function:		GSThreadPool
 *	Explanation:	获取线程池实例
 *	Return:			线程池实例
 */
GSThreadPool* GSThreadPool::GetInstance()
{
	static GSThreadPool*		s_pThreadPool				= NULL;

	if (NULL != s_pThreadPool)
	{
		return s_pThreadPool;
	}

	s_pThreadPool = new GSThreadPool();

	return s_pThreadPool;
}
