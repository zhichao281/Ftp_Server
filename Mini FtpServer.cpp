// Mini FtpServer.cpp : Defines the entry point for the console application.
#include "define.h"
#include "StdAfx.h"
#include "FtpApp.h"

#include "GSThreadPool.h"
#include "thread_pool.h"

#include <iostream>


void fun(std::string s)
{
	std::cout << "thread:" << s<< endl;
	for (int i = 0; i < 20; ++i)
	{
		std::cout << "s:"<<i<<endl;
	}

}

void fun1(int  s,int s2)
{
	std::cout << "thread:" << s <<s2<< endl;


}


// ������������̨����ʼ�ĵط�
void main(void)
{

	netlib::ThreadPool pool(10);
	pool.start();
	int num = 0;
	//while (1)
	//{
	//	char pTest[256];
	//	sprintf(pTest, "hello-%d", num);
	//	std::string str1 = pTest;
	//	pool.append(std::bind(fun, str1));
	//	num++;
	//	if (num>10)
	//	{
	//		break;
	//	}
	//}


	pool.append(std::bind
		(
	[=](int x,int y)
	{
		x++;
		std::cout << "thread:" << x <<y<< endl;
	}, 5, 7));


	//GSThreadPool_AddFun(
	//{
	//	int x=0;
	//	int y=0;
	//	std::cout << "test";	
	//
	//}, NULL);

	//GSThreadPool_AddFun::GetInstance()->Start();


	pool.stop();
	//CFtpAppSingleton->AsynStart();
	system("pause");
	
}



