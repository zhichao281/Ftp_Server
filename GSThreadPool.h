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


#ifndef GSThreadPoolH
#define GSThreadPoolH

#include <list>
#include <functional>

#define DEFAULT_THREAD_COUNT 1


// �߳���
class GSThreadLock
{
public:

	// ���캯��
	GSThreadLock();

	// ��������
	virtual ~GSThreadLock();

public:

	// ��
	void Lock();

	// ����
	void UnLock();

private:

	CRITICAL_SECTION		m_Section;
};

#define GSThreadPool_AddFun(x, y)	GSThreadPool::GetInstance()->AddWork([=](void *)x, y)
#define GSThreadPool_RunNow(x, y)	GSThreadPool::GetInstance()->ExcNow([=](void *)x, y)
#define GSThreadPool_RunNow2(x, y)	GSThreadPool::GetInstance()->ExcNow2([=](void *pData)x, y)

class GSThreadPool
{
	typedef struct _POOL_DATA_
	{
		void*	pThreadData;						// �߳�����
		bool	bAutoDel;							// �Ƿ��Զ�ɾ���߳�����(delete pThreadData)
		std::function<void(void *)>	ThreadFun;		// �̺߳���

	}POOL_DATA, *LPPOOL_DATA;

public:

	// ���캯��
	GSThreadPool();

	// ��������
	~GSThreadPool();

protected:

	// ��ʼ��
	void _Init();

	// ��ն���
	void _ClearQueue();

	// �ر��߳�
	void _CloseThreads();

public:

	// �����̳߳�
	bool Start();

	// ֹͣ
	void Stop();

	// �̳߳��Ƿ�������
	bool IsRunning();

	// ��ӹ���������
	bool AddWork(std::function<void(void *)> Fun, void *pData, bool bPriority = false, bool bAutoDel = false);

	// ����ִ��
	bool ExcNow(std::function<void(void *)> Fun, void *pData, bool bAutoDel = false);

	// ����ִ��
	HANDLE ExcNow2(std::function<void(void *)> Fun, void *pData, bool bAutoDel = false);

public:

	// �Ӷ���ȡ���߳�����
	bool Pop(POOL_DATA &PoolData);

	// ����߳����ݵ�������
	bool Push(POOL_DATA PoolData, bool bPriority = false);

	// �����߳�����
	void Working();

	// �����߳�
	static unsigned long CALLBACK WorkingThread(void *pParam);

	// ���������߳�
	static unsigned long CALLBACK OneWorkThread(void *pParam);

	// ��ȡ�̳߳�ʵ��
	static GSThreadPool* GetInstance();

private:

	bool					m_bStop;							// �Ƿ�ֹͣ
	std::list<POOL_DATA>	m_QueueList;						// ��������
	HANDLE					m_hThread[DEFAULT_THREAD_COUNT];	// �߳̾��
	GSThreadLock			m_Lock;								// �߳���

};

#endif