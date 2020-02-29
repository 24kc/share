#include <iostream>
#include <set>
#include <thread>
#include "my_new.h"
using namespace std;
using namespace akm;

void f();

int main()
{
	const int N = 10;

	thread t[N];
	cout<<_mp;

	for (int i=0; i<N; ++i)
		t[i] = thread(f);

	cout<<_mp;

	for (int i=0; i<N; ++i)
		t[i].join();

	cout<<_mp;
	_mp->check();
}

void
f()
{
	auto s = new set<int>();
	for (int i=0; i<100; ++i)
		s->insert(i);
}
