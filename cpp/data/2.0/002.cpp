#include <iostream>
#include "static_list.h"
using namespace std;
using namespace akm;

typedef struct {
	char data[32];
} Stu;

int main()
{
	static_list<Stu> L;

	L.load("1.akmdb");

	for (auto &x : L)
		cout<<x.data<<endl;

}

