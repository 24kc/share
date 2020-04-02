#include <iostream>
#include <string>
#include <ctime>
#include "thread_pool.h"
using namespace std;

// 测试函数
string f(int n)
{
	// 休眠n个0.1秒
	this_thread::sleep_for(chrono::milliseconds(n*100));
	string s = "ABC";
	for (int i=0; i<n; ++i)
		s += "傻逼";
	return s;
}

int main()
{
	// 线程池, 4个线程
	akm::thread_pool<4> pool;

	constexpr int N = 10;
	future<string> s[N];

	auto t0 = chrono::system_clock::now();

	for (int i=0; i<N; ++i)
		s[i] = pool.push(f, i+1);
	// 向线程池提交N个任务

	cout<<"push "<<N<<" tasks, waiting..."<<endl;

	for (int i=0; i<N; ++i)
		s[i].wait();
	// 等待任务完成

	auto t1 = chrono::system_clock::now();

	chrono::duration<double> t = t1 - t0;
	cout<<"waiting time is "<<t.count()<<"s."<<endl;

	for (int i=0; i<N; ++i)
		cout<<s[i].get()<<endl;
	// 获取计算结果并输出

	return 24-'k';
}

