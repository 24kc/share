#include "first_fit.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif

typedef unsigned char BYTE;

// 记录内存分配信息
typedef struct {
		uint64_t next:48; // 下一个内存块偏移
		uint64_t remain:8; // 内存块剩余大小
		uint64_t :7;
		uint64_t is_used:1; // 是否已分配
} mp_node_t;

#define OFF_NULL  (0)
#define OFF(ptr)  ( (uint64_t) ((BYTE*)ptr - (BYTE*)mp) )
#define PTR(off)  ( (mp_node_t*) ((BYTE*)mp + off) )

#define MP_SIZE  ( sizeof(mempool) )
#define NODE_SIZE  ( sizeof(mp_node_t) )
#define MP_MIN_BLOCK  ( NODE_SIZE + 8 )

#define MP_THROW_OFM(size)  mp_throw_ofm(__FILE__, __LINE__, __func__, "Out of memory.", (size))

#ifdef __cplusplus
extern "C" {
#endif

static void* mp_alloc_nothrow(mempool*, uint64_t);

static void mp_throw_ofm(const char*, uint64_t, const char*, const char*, uint64_t);

static void fprint_uint64(uint64_t, FILE*); // 输出uint64_t

// **  mp_ init alloc/realloc/free  **

mempool*
mp_init(void *mem, size_t size, int flags)
{
	assert(NODE_SIZE == 8);
	assert(mem);
	size &= ~7;

	if ( size < MP_SIZE + NODE_SIZE )
		return NULL;
	mempool *mp = (mempool*)mem;
	mp->flags = flags;
	size -= MP_SIZE + NODE_SIZE;

	mp_node_t *node = (mp_node_t*)&mp[1];
	mp_node_t *end = (mp_node_t*)((BYTE*)node + size);
	end->is_used = 1;
	end->remain = 0;
	end->next = OFF_NULL;

	if ( size < MP_MIN_BLOCK ) {
		mp->first_free = OFF(end);
		mp->nfree = mp->capacity = 0;
		mp->nalloc = 0;
		return mp;
	}

	node->is_used = 0;
	node->next = OFF(end);
	mp->first_free = OFF(node);

	mp->nfree = mp->capacity = OFF(end) - mp->first_free;
	mp->nalloc = 0;
	return mp;
}

void*
mp_alloc(mempool *mp, size_t size)
{
	assert(mp);

	uint64_t block_size = NODE_SIZE + (size & ~7);
	if ( size & 7 || ! size )
		block_size += 8;

	mp_node_t *node = PTR(mp->first_free);
	while ( node->next ) {
		if ( ! node->is_used )
			break;
		node = PTR(node->next);
	}
	mp->first_free = OFF(node);

	while ( node->next ) {
		mp_node_t *next = PTR(node->next);
		if ( node->is_used ) {
			node = next;
			continue;
		}

		if ( ! next->is_used ) {
			do
				next = PTR(next->next);
			while ( ! next->is_used );
			node->next = OFF(next);
		}

		uint64_t cap = node->next - OFF(node);
		if ( cap >= block_size ) {
			node->is_used = 1;
			node->remain = block_size - NODE_SIZE - size;
			cap -= block_size;
			if ( ! cap )
				;
			else if ( cap >= MP_MIN_BLOCK ) {
				mp_node_t *new_node = (mp_node_t*)((BYTE*)node + block_size);
				new_node->is_used = 0;
				new_node->next = node->next;
				node->next = OFF(new_node);
			} else {
				node->remain += 8;
				block_size += 8;
			}
			mp->nalloc += size;
			mp->nfree -= block_size;
			if ( OFF(node) == mp->first_free )
				mp->first_free = node->next;
			return &node[1];
		}

		node = next;
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

	mp_node_t *node = (mp_node_t*)((BYTE*)mem - NODE_SIZE);
	assert(node->is_used);
	uint64_t capacity = node->next - OFF(node);
	uint64_t block_size = NODE_SIZE + (size & ~7);
	if ( size & 7 )
		block_size += 8;
	uint64_t old_size = capacity - NODE_SIZE - node->remain;

	mp_node_t *next = PTR(node->next);

	if ( block_size <= capacity ) {
		node->remain = block_size - NODE_SIZE - size;
		mp->nalloc += size;
		mp->nalloc -= old_size;
		uint64_t cap = capacity - block_size;
		if ( ! cap )
			return mem;
		if ( next->is_used ) {
			if ( cap >= MP_MIN_BLOCK ) {
				mp_node_t *new_node = (mp_node_t*)((BYTE*)node + block_size);
				new_node->is_used = 0;
				new_node->next = node->next;
				node->next = OFF(new_node);
				mp->nfree += cap;
				if ( OFF(new_node) < mp->first_free )
					mp->first_free = OFF(new_node);
			} else
				node->remain += 8;
		} else {
			mp_node_t *new_node = (mp_node_t*)((BYTE*)node + block_size);
			*new_node = *next;
			node->next = OFF(new_node);
			mp->nfree += cap;
			if ( OFF(new_node) < mp->first_free )
				mp->first_free = OFF(new_node);
		}
		return mem;
	}

	// block_size > capacity
	if ( ! next->is_used ) {
		mp_node_t *free_node = next;
		next = PTR(free_node->next);
		if ( ! next->is_used ) {
			do
				next = PTR(next->next);
			while ( ! next->is_used );
			free_node->next = OFF(next);
		}

		uint64_t free_cap = free_node->next - OFF(free_node);
		uint64_t cap = capacity + free_cap;
		if ( cap >= block_size ) {
			node->next = free_node->next;
			mp->nalloc -= old_size;
			mp->nfree += capacity;
			node->remain = block_size - NODE_SIZE - size;
			cap -= block_size;
			if ( ! cap )
				;
			else if ( cap >= MP_MIN_BLOCK ) {
				mp_node_t *new_node = (mp_node_t*)((BYTE*)node + block_size);
				new_node->is_used = 0;
				new_node->next = node->next;
				node->next = OFF(new_node);
			} else {
				node->remain += 8;
				block_size += 8;
			}
			mp->nalloc += size;
			mp->nfree -= block_size;
			if ( OFF(free_node) == mp->first_free )
				mp->first_free = node->next;
			return mem;
		}
	}

	void *new_mem = mp_alloc_nothrow(mp, size);
	if ( ! new_mem ) {
		if ( mp->flags & MP_THROW )
			MP_THROW_OFM(size);
		return NULL;
	}
	memcpy(new_mem, mem, old_size);
	mp_free(mp, mem);

	return new_mem;
}

void
mp_free(mempool *mp, void *mem)
{
	assert(mp);
	assert(mem);

	mp_node_t *node = (mp_node_t*)((BYTE*)mem - NODE_SIZE);
	assert(node->is_used);
	node->is_used = 0;
	if ( OFF(node) < mp->first_free )
		mp->first_free = OFF(node);

	uint64_t cap = node->next - OFF(node);
	mp->nalloc -= cap - NODE_SIZE - node->remain;
	mp->nfree += cap;

	for (;;) {
		mp_node_t *next = PTR(node->next);
		if ( next->is_used )
			break;
		node->next = next->next;
	}
}

// ** mp_ max_block_size alloc_nothrow **

size_t
mp_max_block_size(mempool *mp)
{
	uint64_t max = 0;
	mp_node_t *node = PTR(mp->first_free);

	while ( node->next ) {
		mp_node_t *next = PTR(node->next);
		if ( node->is_used ) {
			node = next;
			continue;
		}

		if ( ! next->is_used ) {
			do
				next = PTR(next->next);
			while ( ! next->is_used );
			node->next = OFF(next);
		}

		uint64_t cap = node->next - OFF(node);
		if ( cap > max )
			max = cap;

		node = next;
	}

	return (size_t)max;
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

// ** mp_ check print **

void
mp_check(mempool *mp)
{
	assert(mp->nalloc < mp->capacity);
	assert(mp->nfree <= mp->capacity);
	assert(mp->nalloc + mp->nfree <= mp->capacity);
	if ( ! mp->capacity )
		return;

	uint64_t end = MP_SIZE + mp->capacity + NODE_SIZE;
	assert(MP_SIZE <= mp->first_free && mp->first_free < end);

	mp_node_t *node = (mp_node_t*)&mp[1];
	while ( node->next ) {
		mp_node_t *next = PTR(node->next);
		if ( node->is_used )
			assert(node->remain <= 16);
		assert(OFF(next) < end);
		node = next;
	}
}

void
mp_print(mempool *mp)
{
	assert(mp);
	FILE *fp = stdout;

	printf("[ ");
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

	printf("|");
	mp_node_t *node = (mp_node_t*)&mp[1];
	while ( node->next ) {
		mp_node_t *next = PTR(node->next);
		uint64_t cap = (BYTE*)next - (BYTE*)node;
		if ( ! node->is_used ) {
			fprint_uint64(cap, fp);
			printf("|");
		} else {
			fprint_uint64(cap - NODE_SIZE - node->remain, fp);
			printf("/");
			fprint_uint64(cap, fp);
			printf("|");
		}
		node = next;
	}
	puts("\n");
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

#ifdef __cplusplus
}
#endif
