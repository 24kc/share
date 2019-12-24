#ifndef _STATIC_LIST_H_
#define _STATIC_LIST_H_

#include "basic_mempool.h"
#include <fstream>
#include <string>

#define __t(T)		template <typename T>
#define NODE(diff)	( (Node*) bmp.getptr( diff ) )
#define INODE(diff)	( (Node*) container->bmp.getptr( diff ) )

namespace akm {
using string = std::string;
using istream = std::istream;
using ostream = std::ostream;
using ifstream = std::ifstream;
using ofstream = std::ofstream;
using ios = std::ios;

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
	const_iterator cbegin() const;
	const_iterator cend() const;
	const_iterator crbegin() const;
	const_iterator crend() const;

	bool empty() const;
	int size() const;

	void clear();
	iterator insert( const_iterator pos, const T& value );
	iterator erase( const_iterator pos );
	void push_back( const T& value );
	void pop_back();
	void push_front( const T& value );
	void pop_front();

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
static_list<T>::cbegin() const
{
	return begin();
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::cend() const
{
	return end();
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::crbegin() const
{
	return rbegin();
}

__t(T)
typename static_list<T>::const_iterator
static_list<T>::crend() const
{
	return rend();
}

__t(T)
bool
static_list<T>::empty() const
{
	return bmp.empty();
}

__t(T)
int
static_list<T>::size() const
{
	return bmp.size();
}

__t(T)
void
static_list<T>::clear()
{
	return bmp.reset();
}

__t(T)
typename static_list<T>::iterator
static_list<T>::insert(const_iterator pos, const T& value)
{
	iterator it(this);

	if ( pos == crend() )
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
static_list<T>::erase(const_iterator pos)
{
	iterator it(this);

	if ( pos == cend() || pos == crend() )
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
	insert(cend(), value);
}

__t(T)
void
static_list<T>::pop_back()
{
	erase(crbegin());
}

__t(T)
void
static_list<T>::push_front( const T& value )
{
	insert(cbegin(), value);
}

__t(T)
void
static_list<T>::pop_front()
{
	erase(cbegin());
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


//////////////
/* iterator */
// functions:

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

