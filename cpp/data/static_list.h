#ifndef _STATIC_LIST_H_
#define _STATIC_LIST_H_

#include "basic_mempool.h"

#define __t(T)		template <typename T>

namespace akm {

__t(T)
class static_list {

	class Node {
		T data;
		int prev;
		int next;
	};

  public:
	static_list();
	~static_list();

  private:
	int head;
	int tail;
	int list_size;

	basic_mempool<Node> bmp;
};

__t(T)
static_list<T>::static_list()
{
	bmp.init(10);
	head = bmp.alloc();
	tail = bmp.alloc();
	((Node*)getptr(head))->prev = OFF_NULL;
	list_size = 0;
}

} // namespace akm;

#endif // _STATIC_LIST_H_

