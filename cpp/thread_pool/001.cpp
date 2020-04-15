#include <iostream>
#include <string>
#include "thread_pool.h"
using namespace std;

mutex mtx;

// 测试函数
void f(int n)
{
	// 休眠n个0.1秒
	{
		lock_guard<mutex> lk(mtx);
		cout<<this_thread::get_id()<<" sleep_for "<<n/10.0<<"s"<<endl;
	}
//	this_thread::sleep_for(chrono::milliseconds(n*100));
}

// 线程池, 4个线程
akm::thread_pool<4> pool;

int main()
{
	int N = 10;

	auto t0 = chrono::steady_clock::now();

	cout<<"提交 "<<N<<" 个任务, 正在提交..."<<endl;
	for (int i=0; i<N; ++i)
		pool.thread(f, i+1);
	// 向线程池提交N个任务
	cout<<"提交成功, 等待任务完成..."<<endl;

	pool.join();
	// 等待任务完成

	auto t1 = chrono::steady_clock::now();

	chrono::duration<double> t = t1 - t0;
	cout<<"\n任务完成, 等待时间是 "<<t.count()<<" 秒."<<endl;
	// 等待时间

	N *= 2;
	cout<<"\n\n";

	cout<<"提交 "<<N<<" 个任务, 正在提交..."<<endl;
	for (int i=0; i<N; ++i)
		pool.thread(f, i+1);
	// 向线程池提交2N个任务
	cout<<"提交成功, 等待任务完成..."<<endl;

	pool.join();
	// 等待任务完成

	auto t2 = chrono::steady_clock::now();

	t = t2 - t1;
	cout<<"\n任务完成, 等待时间是 "<<t.count()<<" 秒."<<endl;
	// 等待时间
}
