#include "mempool.h"

#include <stdlib.h>
#include <assert.h>

#define MP_SIZE ( sizeof(mempool) )
#define NODE_SIZE ( sizeof(mp_node_t) )

size_t min2pow(size_t);
mp_node_t* mem_block_init(void*, int);
mp_node_t* mp_get_buddy(mempool*, mp_node_t*);

void mp_node_init(mp_node_t*, int);

void ml_add_prev(mp_node_t*, mp_node_t*);
void ml_add_next(mp_node_t*, mp_node_t*);
void ml_del_prev(mp_node_t*);
void ml_del_next(mp_node_t*);


mempool*
mp_init(void *mem, int size)
{
	if ( size < MP_SIZE + NODE_SIZE )
		return NULL;

	mempool *mp = (mempool*)mem;
	mp->list = mem + MP_SIZE;
	size -= MP_SIZE;

	int max = min2pow(size) >> 1;

	int n = 0;
	for (int i=MP_MIN_BLOCK; i<=max; i<<=1)
		++n;

	if ( size < max + NODE_SIZE * n ) {
		--n;
		max >>= 1;
	}
	if ( n < 1 )
		n = 1;

	mp->list_num = n;
	size -= NODE_SIZE * n;
	for (int i=0; i<n; ++i)
		mp_node_init(&mp->list[i], MP_MIN_BLOCK<<i);

	int block_size = max;
	void *baseptr = mp->list + mp->list_num;
	mp->begin = baseptr;
	mp->end = baseptr + size;
	for (int i=n-1; i>=0; --i) {
		if ( size >= block_size ) {
			mp_node_t *node = mem_block_init(baseptr, block_size);
			ml_add_next(&mp->list[i], node);
			baseptr += block_size;
			size &= ~block_size;
		}
		block_size >>= 1;
	}

	return mp;
}

void
mp_destroy(mempool *mp)
{
	free(mp);
}


void*
mp_alloc(mempool *mp, int size)
{
	if ( size < 0 )
		return NULL;

	int block_size = min2pow(size + NODE_SIZE);
	if ( block_size < MP_MIN_BLOCK )
		block_size = MP_MIN_BLOCK;

	int n = 0;
	for (int i=MP_MIN_BLOCK; i<block_size; i<<=1)
		if ( ++n >= mp->list_num )
			return NULL;

	int m = n;
	while ( m < mp->list_num ) {
		mp_node_t *head = &mp->list[m];
		if ( head->next ) {
			mp_node_t *node = head->next;
			ml_del_next(head);
			while ( m > n ) {
				node->capacity >>= 1;
				mp_node_t *new_node = mem_block_init((void*)node + node->capacity, node->capacity);
				--head;
				ml_add_next(head, node);
				node = new_node;
				--m;
			}
			ml_add_prev(head, node);
			node->size = size;
			return node + 1;
		}
		++m;
	}
	
	return NULL;
}

void
mp_free(mempool *mp, void *p)
{
	mp_node_t *node = (mp_node_t*)p - 1;
	assert( (void*)node >= mp->begin && ((void*)node + node->capacity) <= mp->end );
	assert( ((void*)node - mp->begin) % node->capacity == 0 );

	ml_del_prev(node->next);
	mp_node_t *buddy = mp_get_buddy(mp, node);
	while ( buddy && buddy->size < 0 ) {
		ml_del_next(buddy->prev);
		mp_node_t *merge = node < buddy ? node : buddy;
		merge->size = -1;
		merge->capacity <<= 1;
		node = merge;
		buddy = mp_get_buddy(mp, node);
	}

	int n = 0;
	for (int i=MP_MIN_BLOCK; i<node->capacity; i<<=1)
		++n;

	ml_add_next(&mp->list[n], node);
}

int
mp_capacity(mempool *mp)
{
	return mp->end - mp->begin;
}

void
mp_node_init(mp_node_t *node, int size)
{
	node->capacity = size;
	node->prev = NULL;
	node->next = NULL;
}

void
ml_add_prev(mp_node_t *p, mp_node_t *p1)
{
	p1->prev = p->prev;
	p1->next = p;
	p->prev = p1;
	if ( p1->prev )
		p1->prev->next = p1;
}

void
ml_add_next(mp_node_t *p, mp_node_t *p1)
{
	p1->next = p->next;
	p1->prev = p;
	p->next = p1;
	if ( p1->next )
		p1->next->prev = p1;
}

void
ml_del_prev(mp_node_t *p)
{
	p->prev = p->prev->prev;
	if ( p->prev )
		p->prev->next = p;
}

void
ml_del_next(mp_node_t *p)
{
	p->next = p->next->next;
	if ( p->next )
		p->next->prev = p;
}


size_t
min2pow(size_t n) { // 不小于n的 最小2的幂
	--n;
	n |= n>>1;
	n |= n>>2;
	n |= n>>4;
	n |= n>>8;
	n |= n>>16;
	return n+1;
}

mp_node_t*
mem_block_init(void *mem, int size)
{
	mp_node_t *p = (mp_node_t*)mem;
	p->size = -1;
	p->capacity = size;
	return p;
}

mp_node_t*
mp_get_buddy(mempool *mp, mp_node_t *node)
{
	int capacity = node->capacity;
	void *buddy;
	if ( ((void*)node - mp->begin) & capacity )
		buddy = (void*)node - capacity;
	else {
		buddy = (void*)node + capacity;
		if ( (void*)buddy + capacity > mp->end )
			return NULL;
	}
	return buddy;
}

