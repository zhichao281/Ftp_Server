#pragma once
#pragma execution_character_set("utf-8")

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include <memory>
#include <functional>

namespace netlib
{
	class ThreadPool
	{
	public:
			typedef std::function<void(void )> ThreadTask;

		ThreadPool(int threadNumber);

		~ThreadPool();

		//������������������
		bool append(ThreadTask task, bool bPriority = false);

		//�����̳߳�
		bool start(void);

		//ֹͣ�̳߳�
		bool stop(void);

	private:

		//�߳���ִ�еĹ�������
		void threadWork(void);

		std::mutex mutex_;                                              //������
		std::condition_variable_any condition_empty_;                   //���������Ϊ��ʱ����������
		std::list<ThreadTask> tasks_;                                         //�������
		bool running_;                                                  //�̳߳��Ƿ�������
		int threadNumber_;                                              //�߳���
		std::vector<std::shared_ptr<std::thread>> threads_;             //���������̶߳���ָ��
	};
}

