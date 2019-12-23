#include <iostream>
#include <typeinfo>
#include "Array.h"
using namespace std;
using namespace akm;

void f(int && i)
{
	i = 1;
}

void g(int && i)
{
	f(move(i));
}

int main()
{
	Array<int> a;
	cout<<typeid(a).name()<<endl;
	cout<<typeid(cout).name()<<endl;
}

