#include <iostream>
#include <string.h>
using namespace std;

int main()
{
	auto p = new char[100];
	strcpy(p, "ABC傻逼");
	cout<<p<<endl;
	delete[] p;
	delete[] p;
	delete[] p;
	delete[] p;
	delete[] p;
}

