#include <iostream>
#include <list>
#include <cstring>
#include "my_new.h" // 重载new
using namespace std;
using akm::mp; // 全局内存池指针

int main()
{
	char a[1200];
	mp = mp_init(a, sizeof(a), MP_THROW);
	// 使用这个内存池作为new的源泉

	mp_print(mp);

	list<int> *L = new list<int>();
	for (int i=0; i<24; ++i)
		L->push_back(i);

	mp_print(mp);
}

