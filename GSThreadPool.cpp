// +----------------------------------------------------------------------
// | Green C++ Basis Components
// +----------------------------------------------------------------------
// | GSThreadLock	�߳���
// | GSThreadPool	�̳߳�
// +----------------------------------------------------------------------
// | Author:	Maga
// +----------------------------------------------------------------------
// | Modify:	Maga
// +----------------------------------------------------------------------


#include "stdafx.h"
#include "GSThreadPool.h"


/*
 *	Function:		GSThreadLock
 *	Explanation:	���캯��
 */
GSThreadLock::GSThreadLock()
{
	InitializeCriticalSection(&m_Section);
}

/*
*	Function:		~GSThreadLock
*	Explanation:	��������
*/
GSThreadLock::~GSThreadLock()
{
	DeleteCriticalSection(&m_Section);
}

/*
 *	Function:		Lock
 *	Explanation:	��
 */
void GSThreadLock::Lock()
{
	EnterCriticalSection(&m_Section);
}

/*
*	Function:		UnLock
*	Explanation:	����
*/
void GSThreadLock::UnLock()
{
	LeaveCriticalSection(&m_Section);
}



/*
 *	Function:		GSThreadPool
 *	Explanation:	���캯��
 */
GSThreadPool::GSThreadPool()
{
	_Init();
	Start();
}

/*
 *	Function:		~GSThreadPool
 *	Explanation:	��������
 */
GSThreadPool::~GSThreadPool()
{
	m_bStop = true;
	_ClearQueue();
	_CloseThreads();
}

/*
 *	Function:		_Init
 *	Explanation:	��ʼ��
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
 *	Explanation:	��ն���
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
 *	Explanation:	�ر��߳�
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
 *	Explanation:	�����̳߳�
 *	Return:			true �ɹ� false ʧ��
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
 *	Explanation:	ֹͣ
 */
void GSThreadPool::Stop()
{
	m_bStop = true;
	_ClearQueue();
	_CloseThreads();
}

/*
 *	Function:		IsRunning
 *	Explanation:	�̳߳��Ƿ�������
 *	Return:			true �������� false ֹͣ
 */
bool GSThreadPool::IsRunning()
{
	return !m_bStop;
}

/*
 *	Function:		AddWork
 *	Explanation:	��ӹ���������
 *	Input:			Fun			����
					pData		����
					bPriority	�Ƿ�Ҫ����ִ��
					bAutoDel	�Ƿ���Ҫ�Զ�delete��pData
 *	Return:			true �ɹ� false ʧ��
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
 *	Explanation:	����ִ��
 *	Input:			Fun			����
					pData		����
					bAutoDel	�Ƿ���Ҫ�Զ�delete��pData
 *	Return:			true �ɹ� false ʧ��
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
*	Explanation:	����ִ��
*	Input:			Fun			����
					pData		����
					bAutoDel	�Ƿ���Ҫ�Զ�delete��pData
*	Return:			���
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
 *	Explanation:	�Ӷ���ȡ���߳�����
 *	Input:			PoolData		�߳�����
 *	Return:			true �ɹ� false ʧ��
 */
bool GSThreadPool::Pop(POOL_DATA &PoolData)
{
	m_Lock.Lock();

	if (0 >= m_QueueList.size())
	{
		// ��ǰ��������Ϊ��
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
 *	Explanation:	����߳����ݵ�������
 *	Input:			PoolData		�߳�����
					bPriority		�Ƿ�����ִ��
 *	Return:			true �ɹ� false ʧ��
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
 *	Explanation:	�����߳�����
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
 *	Explanation:	�����߳�
 *	Input:			pParam		�̲߳���
 *	Return:			S_OK �ɹ� S_FALSE ʧ��
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
 *	Explanation:	���������߳�
 *	Input:			pParam		�̲߳���
 *	Return:			S_OK �ɹ� S_FALSE ʧ��
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
 *	Explanation:	��ȡ�̳߳�ʵ��
 *	Return:			�̳߳�ʵ��
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
