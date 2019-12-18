#include <iostream>
#include <thread>
#include "singleton.h"
using namespace std;
using namespace akm;

void f();

int main()
{
	const int N = 10;

	thread t[N];

	for (int i=0; i<N; ++i)
		t[i] = thread(f);

	for (int i=0; i<N; ++i)
		t[i].join();
}

void
f()
{
	cout<< T::getInstance() <<'\n';
}

