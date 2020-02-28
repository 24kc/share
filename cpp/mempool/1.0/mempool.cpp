#include "mempool.h"

#include <cstring>
#include <cassert>

#define __func__ __PRETTY_FUNCTION__

#define MP_SIZE  ( sizeof(mempool) )
#define NODE_SIZE  ( sizeof(mempool::node_t) )
#define MP_THROW_OFM(size)  mp_throw_ofm(__FILE__, __LINE__, __func__, "Out of memory.", (size))



namespace akm {
using namespace std;
using BYTE = unsigned char;

static int shift2(int, int);
static long min2pow(long);
static void mp_throw_ofm(const char*, int, const char*, const char*, int);

class mempool::node_t {
  public:
	static void
	init(node_t *node, int size);

	node_t() = delete;
	~node_t() = delete;

	static void add_prev(node_t*, node_t*);
	static void add_next(node_t*, node_t*);
	static void del_prev(node_t*);
	static void del_next(node_t*);
	static int prev_size(node_t*);
	static int next_size(node_t*);

	friend ostream& operator<< (ostream &out, const mempool *mp);
	friend ostream& operator<< (ostream &out, const mempool &mp);

//private:
	int size;
	int capacity;
	node_t *prev;
	node_t *next;
};

// ** mempool:: public: **

mempool*
mempool::create(void *mem, int size, int flag)
{
	assert(mem);

	if ( size < (int)(MP_SIZE+NODE_SIZE) )
		return NULL;

	mempool *mp = (mempool*)mem;
	mp->list = (node_t*)((BYTE*)mem + MP_SIZE);
	size -= MP_SIZE;

	int max = min2pow(size) >> 1;
	int n = shift2(MP_MIN_BLOCK, max) + 1;

	if ( size < (int)(max + NODE_SIZE*n) ) {
		--n;
		max >>= 1;
	}
	if ( n < 1 )
		n = 1;

	mp->list_num = n;
	size -= NODE_SIZE * n;
	for (int i=0; i<n; ++i)
		node_t::init(&mp->list[i], MP_MIN_BLOCK<<i);

	int block_size = max;
	void *p = mp->list + mp->list_num;
	mp->begin = p;
	mp->end = (BYTE*)p + size;
	for (int i=n-1; i>=0; --i) {
		if ( size >= block_size ) {
			node_t *node = mb_init(p, block_size);
			node_t::add_next(&mp->list[i], node);
			p = (BYTE*)p + block_size;
			size -= block_size;
		}
		block_size >>= 1;
	}

	mp->flag = flag;
	return mp;
}

void*
mempool::alloc(int size)
{
	assert(this);

	if ( size < 0 )
		return NULL;

	int block_size = min2pow(size + NODE_SIZE);
	if ( block_size < MP_MIN_BLOCK )
		block_size = MP_MIN_BLOCK;

	int n = shift2(MP_MIN_BLOCK, block_size);
	if ( n >= this->list_num ) {
		if ( this->flag & MP_THROW )
			MP_THROW_OFM(size);
		return NULL;
	}

	int m = n;
	while ( m < this->list_num ) {
		node_t *head = &this->list[m];
		if ( head->next ) {
			node_t *node = head->next;
			node_t::del_next(head);
			while ( m > n ) {
				node->capacity >>= 1;
				node_t *new_node = mb_init((BYTE*)node + node->capacity, node->capacity);
				--head;
				node_t::add_next(head, new_node);
				--m;
			}
			node_t::add_prev(head, node);
			node->size = size;
			return node + 1;
		}
		++m;
	}

	if ( this->flag & MP_THROW )
		MP_THROW_OFM(size);
	return NULL;
}

void*
mempool::realloc(void *mem, int size)
{
	assert(this);
	assert(mem);

	if ( size <= 0 ) {
		free(mem);
		return NULL;
	}

	node_t *node = (node_t*)mem - 1;
	assert(node->size >= 0);

	int asize = size + NODE_SIZE;
	int capacity = node->capacity;
	int half_cap = capacity >> 1;

	if ( half_cap < asize && asize <= capacity ) {
		node->size = size;
		return node + 1;
	}

	if ( capacity < asize && asize <= capacity<<1 ) {
		node_t *buddy = mb_get_buddy(this, node);
		if ( buddy && buddy->size < 0 && buddy > node ) {
			node_t::del_next(buddy->prev);
			node->capacity <<= 1;
			node->size = size;
			node_t::del_prev(node->next);
			int n = shift2(MP_MIN_BLOCK, node->capacity);
			node_t::add_prev(&this->list[n], node);
			return node + 1;
		}
	}

	if ( asize <= half_cap ) {
		if ( half_cap < MP_MIN_BLOCK ) {
			node->size = size;
			return node + 1;
		}
		node_t::del_prev(node->next);
		int n = shift2(MP_MIN_BLOCK, half_cap);
		while ( n >= 0 && half_cap >= asize ) {
			node_t *buddy = mb_init((BYTE*)node + half_cap, half_cap);
			node_t::add_next(&this->list[n], buddy);
			half_cap >>= 1;
			--n;
		}
		half_cap <<= 1;
		++n;
		node->size = size;
		node->capacity = half_cap;
		node_t::add_prev(&this->list[n], node);
		return node + 1;
	}

	void *new_block = alloc_nothrow(size);
	if ( ! new_block ) {
		if ( this->flag & MP_THROW )
			MP_THROW_OFM(size);
		return NULL;
	}
	memcpy(new_block, node+1, node->size);
	node->size = size;
	free(node+1);

	return new_block;
}

void
mempool::free(void *mem)
{
	assert(this);
	assert(mem);

	node_t *node = (node_t*)mem - 1;
	assert( (void*)node >= this->begin && ((BYTE*)node + node->capacity) <= (BYTE*)this->end );
	assert( ((BYTE*)node - (BYTE*)this->begin) % node->capacity == 0 );
	assert( node->size >= 0 );

	node_t::del_prev(node->next);
	node_t *buddy = mb_get_buddy(this, node);
	while ( buddy && buddy->size < 0 ) {
		node_t::del_next(buddy->prev);
		node_t *merge = node < buddy ? node : buddy;
		merge->size = -1;
		merge->capacity <<= 1;
		node = merge;
		buddy = mb_get_buddy(this, node);
	}
	node->size = -1;

	int n = shift2(MP_MIN_BLOCK, node->capacity);
	node_t::add_next(&this->list[n], node);
}

int
mempool::capacity() const
{
	return (BYTE*)this->end - (BYTE*)this->begin;
}

int
mempool::max_block_size() const
{
	for (int i=this->list_num-1; i>=0; --i) {
		node_t *head = &this->list[i];
		if ( head->next )
			return head->capacity - NODE_SIZE;
	}
	return 0;
}

void
mempool::check() const
{
	assert((BYTE*)this + MP_SIZE == (BYTE*)this->list);
	assert(this->end >= this->begin);
	assert(this->list_num > 0);
	assert(this->begin == (void*)(this->list + this->list_num));
	for (int i=0; i<this->list_num; ++i) {
		assert(this->list[i].capacity == MP_MIN_BLOCK<<i);
		check_list(&this->list[i]);
	}
}

ostream&
operator<< (ostream &out, const mempool *mp)
{
	using node_t = mempool::node_t;
	int flag = 0;
	out<<"[ "<<mp->capacity()<<" bytes, "<<mp->list_num<<" lists ]\n";
	for (int i=mp->list_num-1; i>=0; --i) {
		int nalloc = node_t::prev_size(&mp->list[i]);
		int nfree = node_t::next_size(&mp->list[i]);
		if ( ! nalloc && ! nfree )
			continue;
		if ( flag )
			out<<"  ";
		flag = 1;
		out<<"{<"<<mp->list[i].capacity<<">";
		if ( nfree )
			out<<"f("<<nfree<<")";
		if ( nalloc ) {
			out<<"a("<<nalloc<<")";
			node_t *ml = &mp->list[i];
			out<<"[";
			while ( ml->prev ) {
				ml = ml->prev;
				out<<ml->size;
				if ( ml->prev )
					out<<",";
			}
			out<<"]";
		}
		out<<"}";
	}
	out<<"\n";
	return out;
}

ostream&
operator<< (ostream &out, const mempool &mp)
{
	return out<<(&mp);
}

// ** mempool:: private: **

void*
mempool::alloc_nothrow(int size) noexcept
{
	int flag = this->flag;
	if ( ! (flag & MP_THROW) )
		return alloc(size);
	this->flag &= ~MP_THROW;
	void *p = alloc(size);
	this->flag = flag;
	return p;
}

mempool::node_t*
mempool::mb_init(void *mem, int size)
{
	node_t *p = (node_t*)mem;
	p->size = -1; 
	p->capacity = size;
	return p;
}

mempool::node_t*
mempool::mb_get_buddy(const mempool *mp, node_t *node)
{
	int capacity = node->capacity;
	node_t *buddy;
	if ( ((BYTE*)node - (BYTE*)mp->begin) & capacity )
		buddy = (node_t*)((BYTE*)node - capacity);
	else {
		buddy = (node_t*)((BYTE*)node + capacity);
		if ( (BYTE*)buddy + capacity > (BYTE*)mp->end )
			return NULL;
	}
	if ( buddy->capacity != node->capacity )
		return NULL;
	return buddy;
}

void
mempool::check_list(node_t *ml)
{
	node_t *p, *p1;

	p = ml; 
	while ( p->next ) {
		p1 = p->next;
		assert(p1->prev == p); 
		assert(p1->capacity == ml->capacity);
		assert(p1->size < 0);
		p = p1;
	}

	p = ml; 
	while ( p->prev ) {
		p1 = p->prev;
		assert(p1->next == p); 
		assert(p1->capacity == ml->capacity);
		assert(0 <= p1->size && p1->size <= (int)(ml->capacity-NODE_SIZE));
		p = p1;
	}
}

// ** node_t:: **

void
mempool::node_t::init(node_t *node, int size) {
	node->capacity = size;
	node->prev = NULL;
	node->next = NULL;
}

void
mempool::node_t::add_prev(node_t *p, node_t *p1)
{
	p1->prev = p->prev;
	p1->next = p;
	p->prev = p1;
	if ( p1->prev )
		p1->prev->next = p1;
}

void
mempool::node_t::add_next(node_t *p, node_t *p1)
{
	p1->next = p->next;
	p1->prev = p;
	p->next = p1;
	if ( p1->next )
		p1->next->prev = p1;
}

void
mempool::node_t::del_prev(node_t *p)
{
	p->prev = p->prev->prev;
	if ( p->prev )
		p->prev->next = p;
}

void
mempool::node_t::del_next(node_t *p)
{
	p->next = p->next->next;
	if ( p->next )
		p->next->prev = p;
}

int
mempool::node_t::prev_size(node_t *ml)
{
	int n = 0;
	while ( ml->prev ) {
		++n; 
		ml = ml->prev;
	}
	return n;
}

int
mempool::node_t::next_size(node_t *ml)
{
	int n = 0;
	while ( ml->next ) {
		++n; 
		ml = ml->next;
	}
	return n;
}

// ** akm:: **

int
shift2(int s, int b)
{
	int n = 0;
	for (int i=s; i<b; i<<=1)
		++n;
	return n;
}

long
min2pow(long n)
{
	--n;
	n |= n>>1;
	n |= n>>2;
	n |= n>>4;
	n |= n>>8;
	n |= n>>16;
	return n+1;
}

void
mp_throw_ofm(const char *file, int line, const char *func, const char *msg, int size)
{
	fprintf(stderr, "\n%s:%d: %s[size = %d]: %s\n", file, line, func, size, msg);
	abort();
}

} // namespace akm

