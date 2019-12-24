#ifndef _STATIC_LIST_H_
#define _STATIC_LIST_H_

#include "basic_mempool.h"

#define __t(T)		template <typename T>
#define NODE(diff)	( (Node*) bmp.getptr( diff ) )

namespace akm {

__t(T)
class static_list {

	struct Node {
		T data;
		int prev;
		int next;
	};

  public:
	static_list();
	~static_list();

	class iterator;

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
	NODE(head)->prev = OFF_NULL;
	NODE(head)->next = tail;
	NODE(tail)->prev = head;
	NODE(tail)->next = OFF_NULL;
	list_size = 0;
}

__t(T)
static_list<T>::~static_list()
{
	bmp.destroy();
}


// iterator
__t(T)
class static_list<T>::iterator {
  public:
	iterator& operator= (const iterator& it);
//	int operator!=
};

} // namespace akm;

#endif // _STATIC_LIST_H_
