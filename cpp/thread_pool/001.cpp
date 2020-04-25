#include "thread_pool.h"
#include <iostream>
#include <string>
using namespace std;

void f(string& s)
{
	this_thread::sleep_for(0.3s);
	cout<<s<<endl;
}

void g(string&& s)
{
	cout<<s<<endl;
	this_thread::sleep_for(0.3s);
	cout<<"g() end"<<endl;
}

int main()
{
	// 线程池, 3个线程
	akm::thread_pool<3> pool;

	string s = "ABC";

	// 向线程池提交任务
	pool.thread(&string::push_back, &s, 'D');
	pool.thread(f, ref(s));

	// 等待任务完成
	pool.join();

	// 向线程池提交任务
	pool.thread(g, s);
	// 不等待任务完成
	// pool.join();
}
