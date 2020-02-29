#include <iostream>
#include <set>
#include "my_new.h"
using namespace std;
using namespace akm;

int main()
{
	cout<<_mp;

	auto s = new set<int>();

	for (int i=0; i<100; ++i)
		s->insert(i);

	cout<<_mp;
}

