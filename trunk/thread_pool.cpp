#pragma once
#pragma execution_character_set("utf-8")

#include "thread_pool.h"
#include <stdio.h>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <windows.h>
#include <iostream>

using namespace netlib;

ThreadPool::ThreadPool(int threadNumber)
:threadNumber_(threadNumber),
running_(true),
threads_(threadNumber_)
{
	threads_.clear();
}

ThreadPool::~ThreadPool()
{
	if (running_)
	{
		stop();
	}
}

bool ThreadPool::start(void)
{
	for (int i = 0; i < threadNumber_; i++)
	{
		threads_.push_back(std::make_shared<std::thread>(std::bind(&ThreadPool::threadWork, this)));//ѭ�������߳�       
	}
	Sleep(500);
	std::cout << "start" << std::endl;

	return true;
}

bool ThreadPool::stop(void)
{
	std::cout << "stop" << std::endl;
	if (running_)
	{
		running_ = false;
		for (auto t : threads_)
		{			
			t->join();  //ѭ���ȴ��߳���ֹ			
		}
	}
	return true;
}


/*
*	Function:		append
*	Explanation:	��ӹ���������
*	Input:			ThreadTask			����

bPriority	�Ƿ�Ҫ����ִ��

*	Return:			true �ɹ� false ʧ��
*/
bool ThreadPool::append(ThreadTask task,  bool bPriority /* = false */)
{
	std::lock_guard<std::mutex> guard(mutex_);
	tasks_.push_front(task);   //������������������
	condition_empty_.notify_one();//����ĳ���߳���ִ�д�����
	return true;
}

void ThreadPool::threadWork(void)
{
	ThreadTask task = nullptr;
	while (running_)
	{
		{
			std::lock_guard<std::mutex> guard(mutex_);
			if (tasks_.empty())
			{
				condition_empty_.wait(mutex_);  //�ȴ���������������
			}
			if (!tasks_.empty())
			{
				task = tasks_.front();  //����������л�ȡ�ʼ����
				tasks_.pop_front();     //��ȡ�ߵ����񵯳��������
			}
			else
			{
				continue;
			}
		}
		task(); //ִ������
	}
}
