#include <iostream>
#include "thread_pool.h"
using namespace std;

int num = 1;

void f1()
{
	while ( num != 1 )
		;
	cout<<'A';
	++num;
}

void f2()
{
	while ( num != 2 )
		;
	cout<<'B';
	++num;
}

void f3()
{
	while ( num != 3 )
		;
	cout<<'C';
	++num;
}

void f4()
{
	while ( num != 4 )
		;
	cout<<"D"<<endl;
	num = 1;
}

int main()
{
	// 线程池, 3个线程
#if 1
	auto& pool = *new akm::thread_pool<3>();
#else
	akm::thread_pool<3> pool;
#endif

	cout.setf(ios_base::unitbuf);
	constexpr int N = 10;

	// 向线程池提交任务
	for (int i=0; i<N; ++i) {
		pool.thread(&f1);
		pool.thread(&f2);
		pool.thread(&f3);
		pool.thread(&f4);
	}
	// 等待任务完成
	pool.join();

	cout<<endl;

	for (int i=0; i<N; ++i) {
		pool.thread(&f1);
		pool.thread(&f2);
		pool.thread(&f3);
		pool.thread(&f4);
	}
	// 不等待任务完成
	// pool.join();
}
