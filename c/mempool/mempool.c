#include "mempool.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

static const uint64_t MP_MIN_BLOCK = 16;
// 2^n >= 16

typedef unsigned char BYTE;

// 记录内存分配信息
typedef struct mp_record_t {
	uint64_t size:48; // 提供的内存大小
	uint64_t index:8; // lists[index], (index:6)
	uint64_t :7;
	uint64_t is_used:1; // 是否已分配
} mp_record_t;

// mempool record node, use offset
typedef struct {
	union {
		uint64_t prev:48;
		mp_record_t record;
	};
	uint64_t next;
} mp_node_t;

#define OFF_NULL  (0L)
#define OFF(ptr)  ( (BYTE*)ptr - (BYTE*)mp )
#define PTR(off)  ( (mp_node_t*) ((BYTE*)mp + off) )

#define MP_SIZE  ( sizeof(mempool) )
#define NODE_SIZE  ( sizeof(mp_node_t) )
#define RECORD_SIZE  ( sizeof(mp_record_t) )

#define MP_THROW_OFM(size)  mp_throw_ofm(__FILE__, __LINE__, __func__, "Out of memory.", (size))

#ifdef __cplusplus
extern "C" {
#endif

static void* mp_alloc_nothrow(mempool*, uint64_t);

static mp_node_t* mp_block_split(mp_node_t*); // 把内存块一分为二
static mp_node_t* mp_get_buddy(mempool*, mp_node_t*); // 返回已分配内存块的伙伴或NULL

static int mp_lists_index(uint64_t); // MP_MIN_BLOCK -> 0
static void mp_node_init(mp_node_t*, int); // 初始化记录结点
static void mp_list_add(mempool*, mp_node_t*, mp_node_t*);
static void mp_list_del(mempool*, mp_node_t*);

static void mp_check_list(mempool*, mp_node_t*);
static void mp_throw_ofm(const char*, uint64_t, const char*, const char*, uint64_t);

static uint64_t min2pow(uint64_t n); // 不小于n的最小2的整数幂
static int int64_highest_bit(uint64_t); // 最高位1是第几位 (0~63)
static void fprint_uint64(uint64_t, FILE*); // 输出uint64_t


// **  mp_ init alloc/realloc/free  **

mempool*
mp_init(void *mem, size_t size, int flags)
{
	assert(NODE_SIZE == 16);
	assert(mem);

	if ( size < MP_SIZE + NODE_SIZE )
		return NULL;

	mempool *mp = (mempool*)mem;
	mp_node_t *lists = (mp_node_t*)&mp[1];
	mp->flags = flags;

	size -= MP_SIZE;
	uint64_t max = min2pow(size) >> 1;
	int n = mp_lists_index(max) + 1;
	// max = 2^x
	// 2^x < size <= 2^(x+1)

	// size >= 16,  max >= 8
	if ( size < max + NODE_SIZE*n ) {
		--n;
		max >>= 1;
	}
	if ( max < MP_MIN_BLOCK )
		max = MP_MIN_BLOCK;
	if ( n < 1 )
		n = 1;

	mp->nlists = n;
	size -= NODE_SIZE * n;
	for (int i=0; i<n; ++i)
		mp_node_init(&lists[i], i);

	uint64_t cap = size;
	mp_node_t *node = (mp_node_t*)&lists[n];
	for (int index=n-1; index>=0; --index) {
		if ( cap >= max ) {
			node->record.is_used = 0;
			mp_list_add(mp, &lists[index], node);
			node = (mp_node_t*)((BYTE*)node + max);
			cap -= max;
		}
		max >>= 1;
	}
/*
	if ( cap >= MP_MIN_BLOCK ) {
		node->is_used = 0;
		mp_list_add(mp, &lists[0], node);
		cap -= MP_MIN_BLOCK;
	}
*/
	mp->nfree = mp->capacity = size - cap;
	mp->nalloc = 0;

	return mp;
}

void*
mp_alloc(mempool *mp, size_t size)
{
	assert(mp);

	// block_size = 2^n  >= size + RECORD_SIZE
	uint64_t block_size = min2pow(size + RECORD_SIZE);

	// alloc(0)返回有效指针
	if ( ! size )
		block_size = MP_MIN_BLOCK;

	mp_node_t *lists = (mp_node_t*)&mp[1];
	int index = mp_lists_index(block_size);
	int i = index;
	while ( i < mp->nlists ) {
		mp_node_t *head = &lists[i];
		if ( head->next ) {
			mp_node_t *node = PTR(head->next);
			mp_list_del(mp, PTR(head->next));
			while ( i > index ) {
				mp_node_t *new_node = mp_block_split(node);
				mp_list_add(mp, --head, new_node);
				--i;
			}
			node->record.is_used = 1;
			node->record.size = size;
			mp->nalloc += node->record.size;
			mp->nfree -= MP_MIN_BLOCK << node->record.index;
			return &node->next;
		}
		++i;
	}
	
	if ( mp->flags & MP_THROW )
		MP_THROW_OFM(size);
	return NULL;
}

void*
mp_realloc(mempool *mp, void *mem, size_t size)
{
	assert(mp);
	assert(mem);

	if ( ! size ) {
		mp_free(mp, mem);
		return NULL;
	}

	mp_node_t *node = (mp_node_t*)((BYTE*)mem - RECORD_SIZE);
	assert(node->record.is_used);

	uint64_t asize = size + RECORD_SIZE;
	uint64_t capacity = MP_MIN_BLOCK << node->record.index;
	uint64_t half_cap = capacity >> 1;

	if ( half_cap < asize && asize <= capacity ) {
		mp->nalloc += size;
		mp->nalloc -= node->record.size;
		node->record.size = size;
		return &node->next;
	}

	if ( capacity < asize && asize <= capacity<<1 ) {
		mp_node_t *buddy = mp_get_buddy(mp, node);
		if ( buddy && buddy > node ) {
			mp->nalloc += size;
			mp->nalloc -= node->record.size;
			mp->nfree -= capacity;
			mp_list_del(mp, buddy);
			++node->record.index;
			node->record.size = size;
			return &node->next;
		}
	}

	mp_node_t *lists = (mp_node_t*)&mp[1];
	if ( asize <= half_cap ) {
		mp->nalloc += size;
		mp->nalloc -= node->record.size;
		if ( half_cap < MP_MIN_BLOCK ) {
			node->record.size = size;
			return &node->next;
		}
		while ( node->record.index > 0 && half_cap >= asize ) {
			mp->nfree += half_cap;
			mp_node_t *buddy = mp_block_split(node);
			mp_list_add(mp, &lists[node->record.index], buddy);
			half_cap >>= 1;
		}
		node->record.size = size;
		return &node->next;
	}

	void *new_mem = mp_alloc_nothrow(mp, size);
	if ( ! new_mem ) {
		if ( mp->flags & MP_THROW )
			MP_THROW_OFM(size);
		return NULL;
	}
	memcpy(new_mem, &node->next, node->record.size);
	mp_free(mp, mem);

	return new_mem;
}

void
mp_free(mempool *mp, void *mem)
{
	assert(mp);
	assert(mem);

	mp_node_t *node = (mp_node_t*)((BYTE*)mem - RECORD_SIZE);
	assert( node->record.is_used );

	node->record.is_used = 0;
	mp->nalloc -= node->record.size;
	mp->nfree += MP_MIN_BLOCK << node->record.index;

	mp_node_t *buddy = mp_get_buddy(mp, node);
	while ( buddy ) {
		mp_list_del(mp, buddy);
		if ( node > buddy )
			node = buddy;
		++node->record.index;
		buddy = mp_get_buddy(mp, node);
	}

	mp_node_t *lists = (mp_node_t*)&mp[1];
	mp_list_add(mp, &lists[node->record.index], node);
}

void*
mp_alloc_nothrow(mempool *mp, uint64_t size)
{
	int flags = mp->flags;
	if ( ! (flags & MP_THROW) )
		return mp_alloc(mp, size);
	mp->flags &= ~MP_THROW;
	void *mem = mp_alloc(mp, size);
	mp->flags = flags;
	return mem;
}

// **  mp_ max_block_size get_buddy  **

uint64_t
mp_max_block_size(mempool *mp)
{
	mp_node_t *lists = (mp_node_t*)&mp[1];
	for (int index=mp->nlists-1; index>=0; --index) {
		mp_node_t *head = &lists[index];
		if ( head->next )
			return (MP_MIN_BLOCK << index) - RECORD_SIZE;
	}
	return 0;
}

mp_node_t*
mp_block_split(mp_node_t *node)
{
	--node->record.index;
	uint64_t half_cap = MP_MIN_BLOCK << node->record.index;
	mp_node_t *new_node = (mp_node_t*)((BYTE*)node + half_cap);
	new_node->record.is_used = 0;
	return new_node;
}

mp_node_t*
mp_get_buddy(mempool *mp, mp_node_t *node)
{
	mp_node_t *lists = (mp_node_t*)&mp[1];
	BYTE *begin = (BYTE*)&lists[mp->nlists];
	BYTE *end = begin + mp->capacity;
	uint64_t capacity = MP_MIN_BLOCK << node->record.index;

	mp_node_t *buddy;
	if ( ((BYTE*)node - begin) & capacity )
		buddy = (mp_node_t*)((BYTE*)node - capacity);
	else {
		buddy = (mp_node_t*)((BYTE*)node + capacity);
		if ( (BYTE*)buddy + capacity > end )
			return NULL;
	}

	if ( buddy->record.is_used || buddy->record.index != node->record.index )
		return NULL;
	return buddy;
}

// ** lists_index, node_init, node_add, node_del

int
mp_lists_index(uint64_t size)
{
	return int64_highest_bit(size) - 4;
}

void
mp_node_init(mp_node_t *node, int index)
{
	node->record.index = index;
	node->prev = OFF_NULL;
	node->next = OFF_NULL;
}

void
mp_list_add(mempool *mp, mp_node_t *node, mp_node_t *new_node)
{
	new_node->record.index = node->record.index;
	new_node->next = node->next;
	new_node->prev = OFF(node);
	node->next = OFF(new_node);
	if ( new_node->next )
		PTR(new_node->next)->prev = OFF(new_node);
}

void
mp_list_del(mempool *mp, mp_node_t *node)
{
	mp_node_t *prev_node = PTR(node->prev);
	prev_node->next = node->next;
	if ( node->next )
		PTR(node->next)->prev = node->prev;
}

// ** mp_ check, check_list, throw_ofm, print **

void
mp_check(mempool *mp)
{
	assert(mp);
	mp_node_t *lists = (mp_node_t*)&mp[1];
	assert(0 < mp->nlists && mp->nlists < 44);
	assert(mp->nalloc < mp->capacity);
	assert(mp->nfree <= mp->capacity);
	assert(mp->nalloc + mp->nfree <= mp->capacity);
	for (int index=0; index<mp->nlists; ++index) {
		assert(lists[index].record.index == index);
		mp_check_list(mp, &lists[index]);
	}
	mp_node_t *node = (mp_node_t*)&lists[mp->nlists];
	BYTE *end = (BYTE*)node + mp->capacity;
	while ( (BYTE*)node < end ) {
		uint64_t cap = MP_MIN_BLOCK << node->record.index;
		assert( node->record.index < (unsigned)mp->nlists );
		if ( node->record.is_used )
			assert(node->record.size + RECORD_SIZE <= cap);
		node = (mp_node_t*)((BYTE*)node + cap);
	}
}

void
mp_check_list(mempool *mp, mp_node_t *list)
{
	mp_node_t *p, *p1;

	p = list;
	assert(p->record.index < (unsigned)mp->nlists);
	while ( p->next ) {
		p1 = PTR(p->next);
		assert(PTR(p1->prev) == p);
		assert( ! p1->record.is_used );
		p = p1;
	}
}

void
mp_throw_ofm(const char *file, uint64_t line, const char *func, const char *msg, uint64_t size)
{
	FILE *fp = stderr;
	fprintf(fp, "mempool: %s:", file);
	fprint_uint64(line, fp);
	fprintf(fp, ": %s[size = ", func);
	fprint_uint64(size, fp);
	fprintf(fp, "]: %s\n", msg);
	abort();
}

void
mp_print(mempool *mp)
{
	assert(mp);
	FILE *fp = stdout;

	printf("[ %d lists, ", mp->nlists);
	fprint_uint64(mp->capacity, fp);
	printf(" total, ");
	fprint_uint64(mp->nfree, fp);
	printf(" free, ");
	fprint_uint64(mp->nalloc, fp);
	printf(" used ]\n");

	if ( ! mp->capacity ) {
		printf("||\n\n");
		return;
	}

	mp_node_t *lists = (mp_node_t*)&mp[1];
	mp_node_t *node = (mp_node_t*)&lists[mp->nlists];
	BYTE *end = (BYTE*)node + mp->capacity;
	printf("|");
	while ( (BYTE*)node < end ) {
		uint64_t cap = MP_MIN_BLOCK << node->record.index;
		if ( ! node->record.is_used ) {
			fprint_uint64(cap, fp);
			printf("|");
		} else {
#if 0
			printf("-");
			fprint_uint64(node->record.size, fp);
			printf("|");
#else
		//	printf("<");
			fprint_uint64(node->record.size, fp);
			printf("/");
			fprint_uint64(cap, fp);
		//	printf(">");
			printf("|");
#endif
		}
		node = (mp_node_t*)((BYTE*)node + cap);
	}
	puts("\n");
}

void
fprint_uint64(uint64_t i, FILE *fp)
{
	char stack[24];
	int sp = 0;
	do {
		stack[sp++] = '0' + (i % 10);
		i /= 10;
	} while ( i );
	while ( --sp >= 0 )
		fputc(stack[sp], fp);
}

// ** min2pow, int64_highest_bit **

uint64_t
min2pow(uint64_t n) { // 不小于n的 最小2的幂
	--n;
	n |= n>>1;
	n |= n>>2;
	n |= n>>4;
	n |= n>>8;
	n |= n>>16;
	n |= n>>32;
	return n+1;
}

int
int64_highest_bit(uint64_t i) // 0 ~ 63
{
	int n = 0;
	if ( i & 0xffffffff00000000 ) {
		n += 32;
		i &= 0xffffffff00000000;
	}
	if ( i & 0xffff0000ffff0000 ) {
		n += 16;
		i &= 0xffff0000ffff0000;
	}
	if ( i & 0xff00ff00ff00ff00 ) {
		n += 8;
		i &= 0xff00ff00ff00ff00;
	}
	if ( i & 0xf0f0f0f0f0f0f0f0 ) {
		n += 4;
		i &= 0xf0f0f0f0f0f0f0f0;
	}
	if ( i & 0xcccccccccccccccc ) {
		n += 2;
		i &= 0xcccccccccccccccc;
	}
	if ( i & 0xaaaaaaaaaaaaaaaa ) {
		n += 1;
	}
	return n;
}

#ifdef __cplusplus
}
#endif

