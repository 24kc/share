#ifndef _STATIC_LIST_H_
#define _STATIC_LIST_H_

#include "basic_mempool.h"
#include <fstream>
#include <string>

#define __t(T)		template <typename T>
#define NODE(diff)	( (Node*) bmp.getptr( diff ) )
#define INODE(diff)	( (Node*) container->bmp.getptr( diff ) )

#define TEST printf("line = %d\n", __LINE__);

namespace akm {
using std::string;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::ios;

__t(T)
class static_list {

	struct Node {
		T data;
		int prev;
		int next;
	};

  public:
	class iterator;

	static_list();
	~static_list();

	iterator begin();
	iterator end();
	iterator rbegin();
	iterator rend();

	bool empty() const;
	int size() const;

	void clear();
	iterator insert( iterator pos, const T& value );
	iterator erase( iterator pos );
	void push_back( const T& value );
	void pop_back();
	void push_front( const T& value );
	void pop_front();

	void sort(bool (*cmp)(const T&, const T&));

	void write( ostream& out ) const;
	void read( istream& in );

	bool save( const string& file_name ) const;
	bool load( const string& file_name );

  private:
	int head;
	int tail;

	basic_mempool<Node> bmp;
};

/* iterator */
__t(T)
class static_list<T>::iterator {
	friend static_list<T>;

  public:
	iterator();
	iterator(const static_list<T>*);

	iterator& operator= (const iterator& other);
	bool operator!= (const iterator& other) const;
	bool operator== (const iterator& other) const;
	iterator& operator++ ();
	iterator& operator-- ();
	iterator operator++ (int);
	iterator operator-- (int);
	T& operator* () const;
	T* operator-> () const;

  private:
	int node;
	const static_list<T> *container;
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
bool
static_list<T>::empty() const
{
	return size() == 0;
}

__t(T)
int
static_list<T>::size() const
{
	return bmp.size() - 2;
}

__t(T)
void
static_list<T>::clear()
{
	return bmp.reset();
}

__t(T)
typename static_list<T>::iterator
static_list<T>::insert(iterator pos, const T& value)
{
	iterator it(this);

	if ( pos == rend() )
		return it;

	auto node = bmp.alloc();
	if ( ! node )
		return it;

	// NODE(node)->data = value;
	new (&NODE(node)->data) T(value); // 原位拷贝构造

	auto prev = NODE(pos.node)->prev;

	NODE(node)->next = pos.node;
	NODE(node)->prev = prev;
	NODE(pos.node)->prev = node;
	NODE(prev)->next = node;

	it.node = node;
	return it;
}

__t(T)
typename static_list<T>::iterator
static_list<T>::erase(iterator pos)
{
	iterator it(this);

	if ( pos == end() || pos == rend() )
		return it;

	auto prev = NODE(pos.node)->prev;
	auto next = NODE(pos.node)->next;

	NODE(next)->prev = prev;
	NODE(prev)->next = next;

	bmp.free(pos.node);

	it.node = next;
	return it;
}

__t(T)
void
static_list<T>::push_back( const T& value )
{
	insert(end(), value);
}

__t(T)
void
static_list<T>::pop_back()
{
	erase(rbegin());
}

__t(T)
void
static_list<T>::push_front( const T& value )
{
	insert(begin(), value);
}

__t(T)
void
static_list<T>::pop_front()
{
	erase(begin());
}

__t(T)
void
static_list<T>::write( ostream& out ) const
{
	out.write((const char*)this, sizeof(*this));
	bmp.write(out);
}

__t(T)
void
static_list<T>::read( istream& in )
{
	in.read((char*)this, sizeof(*this));
	bmp.init();
	bmp.read(in);
}

__t(T)
bool
static_list<T>::save( const string& file_name ) const
{
	ofstream out(file_name, ios::binary | ios::trunc);
	if ( ! out.is_open() )
		return 0;
	write(out);
	out.close();
	return 1;
}

__t(T)
bool
static_list<T>::load( const string& file_name )
{
	ifstream in(file_name, ios::binary);
	if ( ! in.is_open() )
		return 0;
	read(in);
	in.close();
	return 1;
}

__t(T)
void
static_list<T>::sort(bool (*cmp)(const T&, const T&))
{
	int sz = size();
	if ( sz < 2 )
		return;

	using TP = T*;
	auto array = new TP[sz];

	auto it = begin();
	for (int i=0; i<sz; ++i) {
		array[i] = &(*it);
		++it;
	}

	qsort(array, sz, sizeof(T*), [cmp](const void* x, const void* y)
	{
		T& a = **(T**)x;
		T& b = **(T**)y;
		return cmp(b, a) - cmp(a, b);
	});

	auto base = (Node*)bmp.baseptr();

	int node = (Node*)array[0] - base;
	NODE(head)->next = node;
	NODE(node)->prev = head;
	NODE(node)->next = (Node*)array[1] - base;

	node = (Node*)array[sz-1] - bmp.baseptr();
	NODE(tail)->prev = node;
	NODE(node)->next = tail;
	NODE(node)->prev = (Node*)array[sz-2] - base;

	for (int i=0; i<sz-1; ++i) {
		node = (Node*)array[i] - base;
		NODE(node)->prev = (Node*)array[i-1] - base;
		NODE(node)->next = (Node*)array[i+1] - base;
	}

	delete[] array;
}


//////////////
/* iterator */
// functions:

__t(T)
static_list<T>::iterator::iterator()
{
}

__t(T)
static_list<T>::iterator::iterator(const static_list<T> *container)
{
	this->container = container;
}

__t(T)
typename static_list<T>::iterator&
static_list<T>::iterator::operator= (const iterator& other)
{
	node = other.node;
	container = other.container;
	return *this;
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
static_list<T>::iterator::operator* () const
{
	return INODE(node)->data;
}

__t(T)
T*
static_list<T>::iterator::operator-> () const
{
	return &INODE(node)->data;
}

} // namespace akm;

#undef __t
#undef NODE
#undef INODE

#undef TEST

#endif // _STATIC_LIST_H_

