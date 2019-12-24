#ifndef _STATIC_LIST_H_
#define _STATIC_LIST_H_

#include "basic_mempool.h"

#define __t(T)		template <typename T>
#define NODE(diff)	( (Node*) bmp.getptr( diff ) )
#define INODE(diff)	( (Node*) container->bmp.getptr( diff ) )

namespace akm {

__t(T)
class static_list {

	struct Node {
		T data;
		int prev;
		int next;
	};

  public:
	class iterator;
	using const_iterator = const iterator;

	static_list();
	~static_list();

	iterator begin();
	iterator end();
	iterator rbegin();
	iterator rend();
	const_iterator cbegin();
	const_iterator cend();
	const_iterator crbegin();
	const_iterator crend();

	bool empty();
	int size();

	void clear();
	iterator insert(const_iterator pos, const T& value);

  private:
	int head;
	int tail;

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
}

__t(T)
static_list<T>::~static_list()
{
	bmp.destroy();
}

__t(T)
typename static_list<T>::iterator
static_list<T>::begin()
{
	iterator it(this);
	it.node = NODE(head)->next;
	return it;
}

__t(T)
typename static_list<T>::iterator
static_list<T>::end()
{
	iterator it(this);
	it.node = tail;
	return it;
}

__t(T)
typename static_list<T>::iterator
static_list<T>::rbegin()
{
	iterator it(this);
	it.node = NODE(tail)->prev;
	return it;
}

__t(T)
typename static_list<T>::iterator
static_list<T>::rend()
{
	iterator it(this);
	it.node = head;
	return it;
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::cbegin()
{
	return begin();
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::cend()
{
	return end();
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::crbegin()
{
	return rbegin();
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::crend()
{
	return rend();
}

__t(T)
bool
static_list<T>::empty()
{
	return bmp.empty();
}

__t(T)
int
static_list<T>::size()
{
	return bmp.size();
}


//////////////
/* iterator */

__t(T)
class static_list<T>::iterator {
  public:
	iterator();
	iterator(static_list<T>*);

	iterator& operator= (const iterator& other);
	bool operator!= (const iterator& other) const;
	bool operator== (const iterator& other) const;
	iterator& operator++ ();
	iterator& operator-- ();
	iterator operator++ (int);
	iterator operator-- (int);
	T& operator* ();
	T* operator-> ();

  private:
	int node;
	static_list<T> *container;
};

__t(T)
static_list<T>::iterator::iterator()
{
}

__t(T)
static_list<T>::iterator::iterator(static_list<T> *container)
{
	this->container = container;
}

__t(T)
typename static_list<T>::iterator&
static_list<T>::iterator::operator= (const iterator& other)
{
	node = other.node;
	container = other.container;
}

__t(T)
bool
static_list<T>::iterator::operator!= (const iterator& other) const
{
	return node != other.node;
}

__t(T)
bool
static_list<T>::iterator::operator== (const iterator& other) const
{
	return node == other.node;
}

__t(T)
typename static_list<T>::iterator&
static_list<T>::iterator::operator++ ()
{
	node = INODE(node)->next;
	return *this;
}

__t(T)
typename static_list<T>::iterator&
static_list<T>::iterator::operator-- ()
{
	node = INODE(node)->prev;
	return *this;
}

__t(T)
typename static_list<T>::iterator
static_list<T>::iterator::operator++ (int)
{
	auto it = *this;
	++(*this);
	return it;
}

__t(T)
typename static_list<T>::iterator
static_list<T>::iterator::operator-- (int)
{
	auto it = *this;
	--(*this);
	return it;
}

__t(T)
T&
static_list<T>::iterator::operator* ()
{
	return INODE(node)->data;
}

__t(T)
T*
static_list<T>::iterator::operator-> ()
{
	return &INODE(node)->data;
}

} // namespace akm;

#undef __t
#undef NODE
#undef INODE

#endif // _STATIC_LIST_H_

