#include <iostream>
#include "mempool.h"
using namespace std;
using namespace akm;

#define N  (1200)

int main()
{
	auto mp = mempool::create(new char[N], N, MP_THROW);
	cout<<sizeof(*mp)<<endl;

	mp->alloc(100);

	cout<<mp;

	mp->check();
	delete[] (char*)mp;
}

