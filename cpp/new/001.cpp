#include <iostream>
#include <list>
#include <string>
#include "my_new.h" // 重载new
using namespace std;
using akm::mp; // 全局内存池指针

int main()
{
	char a[2210];
	mp = mp_init(a, sizeof(a), MP_THROW);
	// 使用这个内存池作为new的源泉

	mp_print(mp);

	auto L = new list<string>();
	for (int i='A'; i<='K'; ++i)
		L->push_back(string(16, i));
	mp_print(mp);

	for (auto &&x : *L)
		cout<<x<<' ';
	cout<<"\n\n";

	L->clear();
	mp_print(mp);

	delete L;
	mp_print(mp);
}

