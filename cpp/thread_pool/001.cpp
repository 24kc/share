#include <iostream>
#include <string>
#include <ctime>
#include "thread_pool.h"
using namespace std;

string f(int n)
{
	this_thread::sleep_for(chrono::milliseconds(n*100));
	string s = "ABC";
	for (int i=0; i<n; ++i)
		s += "傻逼";
	return s;
}

int main()
{
	akm::thread_pool<4> pool;

	constexpr int N = 10;
	future<string> s[N];

	clock_t t0 = clock();

	for (int i=0; i<N; ++i)
		s[i] = pool.push(f, i+1);

	cout<<"push "<<N<<" tasks, waiting..."<<endl;

	for (int i=0; i<N; ++i)
		s[i].wait();

	clock_t t1 = clock();

	cout<<"waiting time is "<<t1-t0<<endl;

	for (int i=0; i<N; ++i)
		cout<<s[i].get()<<endl;

	return 24-'k';
}

