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

	Stu stu;
	for (int i=0; i<100; ++i) {
		sprintf(stu.data, "ABC is sb %d !", i);
		L.push_back(stu);
	}

	L.save("1.akmdb");

}

