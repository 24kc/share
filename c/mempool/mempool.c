#include "mempool.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

#define MP_SIZE ( sizeof(mempool) )
#define NODE_SIZE ( sizeof(mp_node_t) )

#define _MP_THROW_OFM(size)  mp_throw_ofm(__FILE__, __LINE__, __func__, "Out of memory.", size)
#define MP_THROW_OFM(size)  _MP_THROW_OFM((size))

int shift2(int, int);
size_t min2pow(size_t);
mp_node_t* mem_block_init(void*, int);
mp_node_t* mp_get_buddy(mempool*, mp_node_t*);

void mp_node_init(mp_node_t*, int);

void ml_add_prev(mp_node_t*, mp_node_t*);
void ml_add_next(mp_node_t*, mp_node_t*);
void ml_del_prev(mp_node_t*);
void ml_del_next(mp_node_t*);

void mp_check_list(mp_node_t*);
void mp_throw_ofm(const char*, int, const char*, const char*, int);

mempool*
mp_init(void *mem, int size)
{
	if ( size < MP_SIZE + NODE_SIZE )
		return NULL;

	mempool *mp = (mempool*)mem;
	mp->list = mem + MP_SIZE;
	size -= MP_SIZE;

	int max = min2pow(size) >> 1;
	int n = shift2(MP_MIN_BLOCK, max) + 1;

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
			size -= block_size;
		}
		block_size >>= 1;
	}
	if ( size >= MP_MIN_BLOCK ) {
		mp_node_t *node = mem_block_init(baseptr, MP_MIN_BLOCK);
		ml_add_next(&mp->list[0], node);
	}

	mp->nothrow = 1;
	return mp;
}

void*
mp_alloc(mempool *mp, int size)
{
	if ( size < 0 )
		return NULL;

	int block_size = min2pow(size + NODE_SIZE);
	if ( block_size < MP_MIN_BLOCK )
		block_size = MP_MIN_BLOCK;

	int n = shift2(MP_MIN_BLOCK, block_size);
	if ( n >= mp->list_num ) {
		if ( ! mp->nothrow )
			MP_THROW_OFM(size);
		return NULL;
	}

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
				ml_add_next(head, new_node);
				--m;
			}
			ml_add_prev(head, node);
			node->size = size;
			return node + 1;
		}
		++m;
	}
	
	if ( ! mp->nothrow )
		MP_THROW_OFM(size);
	return NULL;
}

void*
mp_realloc(mempool *mp, void *p, int size)
{
	if ( size <= 0 ) {
		mp_free(mp, p);
		return NULL;
	}

	mp_node_t *node = (mp_node_t*)p - 1;
	assert(node->size >= 0);

	size += NODE_SIZE;
	int capacity = node->capacity;
	int half_cap = capacity >> 1;

	if ( size > half_cap && size <= capacity ) {
		node->size = size - NODE_SIZE;
		return node + 1;
	}

	if ( size > capacity && size <= capacity<<1 ) {
		mp_node_t *buddy = mp_get_buddy(mp, node);
		if ( buddy && buddy->size < 0 && buddy > node ) {
			ml_del_next(buddy->prev);
			node->capacity <<= 1;
			node->size = size - NODE_SIZE;
			ml_del_prev(node->next);
			int n = shift2(MP_MIN_BLOCK, node->capacity);
			ml_add_prev(&mp->list[n], node);
			return node + 1;
		}
	}

	void *new_block;
	if ( size <= half_cap ) {
		if ( half_cap < MP_MIN_BLOCK ) {
			node->size = size - NODE_SIZE;
			return node + 1;
		}
		new_block = mp_alloc_nothrow(mp, size - NODE_SIZE);
		if ( ! new_block ) {
			mp_node_t *buddy = mem_block_init((void*)node + half_cap, half_cap);
			int n = shift2(MP_MIN_BLOCK, half_cap);
			ml_add_next(&mp->list[n], buddy);
			node->capacity = half_cap;
			node->size = size - NODE_SIZE;
			ml_del_prev(node->next);
			ml_add_prev(&mp->list[n], node);
			return node + 1;
		}
	} else {
		new_block = mp_alloc_nothrow(mp, size - NODE_SIZE);
		if ( ! new_block ) {
			if ( ! mp->nothrow )
				MP_THROW_OFM(size - NODE_SIZE);
			return NULL;
		}
	}

	memcpy(new_block, node+1, size - NODE_SIZE);
	mp_free(mp, node+1);
	return new_block;
}

void
mp_free(mempool *mp, void *p)
{
	mp_node_t *node = (mp_node_t*)p - 1;
	assert( (void*)node >= mp->begin && ((void*)node + node->capacity) <= mp->end );
	assert( ((void*)node - mp->begin) % node->capacity == 0 );
	assert( node->size >= 0 );

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
	node->size = -1;

	int n = shift2(MP_MIN_BLOCK, node->capacity);
	ml_add_next(&mp->list[n], node);
}

int
mp_capacity(mempool *mp)
{
	return mp->end - mp->begin;
}

void*
mp_max_block(mempool *mp, int *p_size, int flag)
{
	for (int i=mp->list_num-1; i>=0; --i) {
		mp_node_t *head = &mp->list[i];
		if ( head->next ) {
			int max = head->capacity - NODE_SIZE;
			if ( ! flag ) {
				*p_size = max;
				return NULL;
			}
			mp_node_t *node = head->next;
			ml_del_next(head);
			node->size = max;
			ml_add_prev(head, node);
			*p_size = max;
			return node+1;
		}
	}
	*p_size = 0;
	return NULL;
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

int
shift2(int s, int b)
{
	int n = 0;
	for (int i=s; i<b; i<<=1)
		++n;
	return n;
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

void
mp_check(mempool *mp)
{
	assert((void*)mp + MP_SIZE == (void*)mp->list);
	assert(mp->end >= mp->begin);
	assert(mp->list_num > 0);
	assert(mp->begin == (void*)(mp->list + mp->list_num));
	for (int i=0; i<mp->list_num; ++i) {
		assert(mp->list[i].capacity == MP_MIN_BLOCK<<i);
		mp_check_list(&mp->list[i]);
	}
}

void
mp_check_list(mp_node_t *ml)
{
	mp_node_t *p, *p1;

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
		assert(0 <= p1->size && p1->size <= ml->capacity - NODE_SIZE);
		p = p1;
	}
}

void
mp_throw_ofm(const char *file, int line, const char *func, const char *msg, int size)
{
	fprintf(stderr, "mempool: %s:%d: %s[size = %d]: %s\n", file, line, func, size, msg);
	abort();
}

void*
mp_alloc_nothrow(mempool *mp, int size)
{
	int flag = mp->nothrow;
	if ( flag )
		return mp_alloc(mp, size);
	mp->nothrow = 1;
	void *p = mp_alloc(mp, size);
	mp->nothrow = flag;
	return p;
}

void*
mp_realloc_nothrow(mempool *mp, void *p, int size)
{
	int flag = mp->nothrow;
	if ( flag )
		return mp_realloc(mp, p, size);
	mp->nothrow = 1;
	void *p1 = mp_realloc(mp, p, size);
	mp->nothrow = flag;
	return p1;
}


//////////////////// mp_print  /////////////////////

static int
list_prev_num(mp_node_t *ml)
{
	int n = 0;
	while ( ml->prev ) {
		++n;
		ml = ml->prev;
	}
	return n;
}

static int
list_next_num(mp_node_t *ml)
{
	int n = 0;
	while ( ml->next ) {
		++n;
		ml = ml->next;
	}
	return n;
}

void
mp_print(mempool *mp)
{
	int flag = 0;
	printf("[ %d bytes, %d lists ]\n", mp_capacity(mp), mp->list_num);
	for (int i=mp->list_num-1; i>=0; --i) {
		int nalloc = list_prev_num(&mp->list[i]);
		int nfree = list_next_num(&mp->list[i]);
		if ( ! nalloc && ! nfree )
			continue;
		if ( flag )
			printf("  ");
		flag = 1;
		printf("{<%d>", mp->list[i].capacity);
		if ( nfree )
			printf("f(%d)", nfree);
		if ( nalloc ) {
			printf("a(%d)", nalloc);
			mp_node_t *ml = &mp->list[i];
			printf("[");
			while ( ml->prev ) {
				ml = ml->prev;
				printf("%d", ml->size);
				if ( ml->prev )
					printf(",");
			}
			printf("]");
		}
		printf("}");
	}
	puts("");
}

