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


#ifndef GSThreadPoolH
#define GSThreadPoolH

#include <list>
#include <functional>

#define DEFAULT_THREAD_COUNT 1


// 线程锁
class GSThreadLock
{
public:

	// 构造函数
	GSThreadLock();

	// 析构函数
	virtual ~GSThreadLock();

public:

	// 锁
	void Lock();

	// 解锁
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
		void*	pThreadData;						// 线程数据
		bool	bAutoDel;							// 是否自动删除线程数据(delete pThreadData)
		std::function<void(void *)>	ThreadFun;		// 线程函数

	}POOL_DATA, *LPPOOL_DATA;

public:

	// 构造函数
	GSThreadPool();

	// 析构函数
	~GSThreadPool();

protected:

	// 初始化
	void _Init();

	// 清空队列
	void _ClearQueue();

	// 关闭线程
	void _CloseThreads();

public:

	// 启动线程池
	bool Start();

	// 停止
	void Stop();

	// 线程池是否在运行
	bool IsRunning();

	// 添加工作到队列
	bool AddWork(std::function<void(void *)> Fun, void *pData, bool bPriority = false, bool bAutoDel = false);

	// 马上执行
	bool ExcNow(std::function<void(void *)> Fun, void *pData, bool bAutoDel = false);

	// 马上执行
	HANDLE ExcNow2(std::function<void(void *)> Fun, void *pData, bool bAutoDel = false);

public:

	// 从队列取出线程数据
	bool Pop(POOL_DATA &PoolData);

	// 添加线程数据到队列中
	bool Push(POOL_DATA PoolData, bool bPriority = false);

	// 工作线程主体
	void Working();

	// 工作线程
	static unsigned long CALLBACK WorkingThread(void *pParam);

	// 单个工作线程
	static unsigned long CALLBACK OneWorkThread(void *pParam);

	// 获取线程池实例
	static GSThreadPool* GetInstance();

private:

	bool					m_bStop;							// 是否停止
	std::list<POOL_DATA>	m_QueueList;						// 工作队列
	HANDLE					m_hThread[DEFAULT_THREAD_COUNT];	// 线程句柄
	GSThreadLock			m_Lock;								// 线程锁

};

#endif