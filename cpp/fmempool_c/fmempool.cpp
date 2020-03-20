#include "fmempool.h"

#include <stdlib.h>
#include <assert.h>
#include <iostream>

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#else
#define __func__ __PRETTY_FUNCTION__
#endif

namespace akm {

namespace c {

static const uint64_t FMP_MIN_BLOCK = 16;
// 2^n >= 16

#define FMP_BUFSIZ  (1024 * 4)
#define NODE_SIZE  ( sizeof(fmp_node_t) )
#define RECORD_SIZE  ( sizeof(fmp_record_t) )

#define FMP_THROW_OFM(size)  fmp_throw_ofm(__FILE__, __LINE__, __func__, "Out of memory.", (size))

#if 1

int fmp_head_read(fmempool*);
int fmp_head_write(const fmempool*);
int fmp_expand(fmempool*, uint64_t); // 扩展内存
int fmp_reserve(fmempool*, uint64_t); // 确保有某个大小的内存块

fmp_off_t fmp_alloc_nothrow(fmempool*, uint64_t);
uint64_t fmp_max_block_size(fmempool*);

fmp_off_t fmp_get_buddy(fmempool*, fmp_off_t, uint64_t); // 返回已分配内存块的伙伴或NULL

int fmp_lists_index(uint64_t); // FMP_MIN_BLOCK -> 0
void fmp_list_add(fmempool*, fmp_off_t, fmp_off_t);
void fmp_list_del(fmempool*, fmp_off_t);

void fmp_check_list(const fmempool*, const fmp_node_t*);
void fmp_throw_ofm(const char*, uint64_t, const char*, const char*, uint64_t);

int64_t min2pow(int64_t n); // 不小于n的最小2的整数幂
int int64_highest_bit(uint64_t); // 最高位1是第几位 (0~63)

#else

static int fmp_head_read(fmempool*);
static int fmp_head_write(const fmempool*);
static int fmp_expand(fmempool*, uint64_t); // 扩展内存
static int fmp_reserve(fmempool*, uint64_t); // 确保有某个大小的内存块

static fmp_off_t fmp_alloc_nothrow(fmempool*, uint64_t);
static uint64_t fmp_max_block_size(fmempool*);

static fmp_off_t fmp_get_buddy(fmempool*, fmp_off_t, uint64_t); // 返回已分配内存块的伙伴或NULL

static int fmp_lists_index(uint64_t); // FMP_MIN_BLOCK -> 0
static void fmp_list_add(fmempool*, fmp_off_t, fmp_off_t);
static void fmp_list_del(fmempool*, fmp_off_t);

static void fmp_check_list(const fmempool*, const fmp_node_t*);
static void fmp_throw_ofm(const char*, uint64_t, const char*, const char*, uint64_t);

static int64_t min2pow(int64_t n); // 不小于n的最小2的整数幂
static int int64_highest_bit(uint64_t); // 最高位1是第几位 (0~63)

#endif

// **  fmp_ init/close alloc/realloc/free  **

fmempool*
fmp_init(std::fstream *fp, uint64_t size, int flags)
{
	assert(NODE_SIZE == 16);
	assert(fp);

	fmempool *fmp = (fmempool*)malloc(sizeof(fmempool));
	if ( ! fmp )
		goto _init_err;
	fmp->buf = malloc(FMP_BUFSIZ);
	if ( ! fmp->buf )
		goto _init_err;

	fmp->fp = fp;
	uint64_t block_size;
	block_size = min2pow(size);

	fmp_head_t *head;
	head = &fmp->head;
	if ( flags & FMP_CREAT ) {
		if ( block_size < FMP_MIN_BLOCK )
			block_size = FMP_MIN_BLOCK;
		head->flags = flags;
		head->nlists = FMP_NLISTS;
		const uint64_t off = sizeof(fmp_head_t) + sizeof(fmp_node_t) * head->nlists;
		head->begin = head->end = off;
		fmp_node_t *lists = fmp->lists;
		for (int i=0; i<head->nlists; ++i) {
			lists[i].record.index = i;
			lists[i].prev = OFF_NULL;
			lists[i].next = OFF_NULL;
		}
		head->nalloc = 0;
		head->nfree = 0;
		if ( ! fmp_head_write(fmp) )
			goto _init_err;
		if ( ! fmp_reserve(fmp, block_size) )
			goto _init_err;
		return fmp;
	}

	if ( ! fmp_head_read(fmp) )
		goto _init_err;
	uint64_t capacity;
	capacity = head->end - head->begin;
	assert(capacity);
	if ( block_size && ! fmp_reserve(fmp, block_size) )
		goto _init_err;
	return fmp;

_init_err:
	if ( fmp ) {
		if ( fmp->buf )
			free(fmp->buf);
		free(fmp);
	}
	return NULL;
}

void
fmp_close(fmempool *fmp)
{
	assert(fmp_head_write(fmp));
	std::fstream *fs = fmp->fp;
	fs->flush();
	assert( ! fs->bad() );
	fmp->fp = NULL;
	free(fmp->buf);
	fmp->buf = NULL;
	free(fmp);
}

fmp_off_t
fmp_alloc(fmempool *fmp, uint64_t size)
{
	assert(fmp);

	// block_size = 2^n  >= size + RECORD_SIZE
	uint64_t block_size = min2pow(size + RECORD_SIZE);

	// alloc(0)返回有效偏移
	if ( ! size )
		block_size = FMP_MIN_BLOCK;

	fmp_head_t *head = &fmp->head;
	fmp_node_t *lists = fmp->lists;
	int index = fmp_lists_index(block_size);
	int i = index;
	while ( i < head->nlists ) {
		fmp_node_t *list = &lists[i];
		if ( list->next ) {
			fmp_off_t offset = list->next;
			fmp_list_del(fmp, offset);
			fmp_node_t node;
			node.record.index = list->record.index;
			while ( i > index ) {
				--node.record.index;
				uint64_t half_cap = FMP_MIN_BLOCK << node.record.index;
				fmp_off_t new_offset = offset + half_cap;
				fmp_list_add(fmp, node.record.index+1, new_offset);
				--i;
			}
			node.record.is_used = 1;
			node.record.size = size;
			head->nalloc += node.record.size;
			head->nfree -= FMP_MIN_BLOCK << node.record.index;
			assert(fmp_write(fmp, offset, &node, RECORD_SIZE));
			return offset + RECORD_SIZE;
		}
		++i;
	}
	
	if ( ! (head->flags & FMP_NOADD) ) {
		if ( fmp_reserve(fmp, block_size) )
			return fmp_alloc(fmp, size);
	}

	if ( head->flags & FMP_THROW )
		FMP_THROW_OFM(size);
	return OFF_NULL;
}

fmp_off_t
fmp_realloc(fmempool *fmp, fmp_off_t offset, uint64_t size)
{
	assert(fmp);
	assert(offset);

	if ( ! size ) {
		fmp_free(fmp, offset);
		return OFF_NULL;
	}
	offset -= RECORD_SIZE;

	fmp_node_t node;
	assert(fmp_read(fmp, offset, &node, RECORD_SIZE));
	assert(node.record.is_used);

	fmp_head_t *head = &fmp->head;

	uint64_t asize = size + RECORD_SIZE;
	uint64_t capacity = FMP_MIN_BLOCK << node.record.index;
	uint64_t half_cap = capacity >> 1;

	if ( half_cap < asize && asize <= capacity ) {
		head->nalloc += size;
		head->nalloc -= node.record.size;
		node.record.size = size;
		assert(fmp_write(fmp, offset, &node, RECORD_SIZE));
		return offset + RECORD_SIZE;
	}

	if ( capacity < asize && asize <= capacity<<1 ) {
		fmp_off_t buddy = fmp_get_buddy(fmp, offset, node.record.index);
		if ( buddy && buddy > offset ) {
			head->nalloc += size;
			head->nalloc -= node.record.size;
			head->nfree -= capacity;
			fmp_list_del(fmp, buddy);
			++node.record.index;
			node.record.size = size;
			assert(fmp_write(fmp, offset, &node, RECORD_SIZE));
			return offset + RECORD_SIZE;
		}
	}

	if ( asize <= half_cap ) {
		head->nalloc += size;
		head->nalloc -= node.record.size;
		if ( half_cap < FMP_MIN_BLOCK ) {
			node.record.size = size;
			assert(fmp_write(fmp, offset, &node, RECORD_SIZE));
			return offset + RECORD_SIZE;
		}
		while ( node.record.index > 0 && half_cap >= asize ) {
			fmp_off_t new_offset = offset + half_cap;
			fmp_list_add(fmp, node.record.index--, new_offset);
			head->nfree += half_cap;
			half_cap >>= 1;
		}
		node.record.size = size;
		assert(fmp_write(fmp, offset, &node, RECORD_SIZE));
		return offset + RECORD_SIZE;
	}

	fmp_off_t new_offset = fmp_alloc_nothrow(fmp, size);
	if ( ! new_offset ) {
		if ( head->flags & FMP_THROW )
			FMP_THROW_OFM(size);
		return OFF_NULL;
	}
	fmp_memcpy(fmp, new_offset, offset+RECORD_SIZE, node.record.size);
	fmp_free(fmp, offset+RECORD_SIZE);

	return new_offset;
}

void
fmp_free(fmempool *fmp, fmp_off_t offset)
{
	assert(fmp);
	assert(offset);

	offset -= RECORD_SIZE;
	fmp_node_t node;
	assert(fmp_read(fmp, offset, &node, RECORD_SIZE));
	assert(node.record.is_used);
	node.record.is_used = 0;

	fmp_head_t *head = &fmp->head;
	head->nalloc -= node.record.size;
	head->nfree += FMP_MIN_BLOCK << node.record.index;

	fmp_off_t buddy;
	while ( (buddy = fmp_get_buddy(fmp, offset, node.record.index)) ) {
		fmp_list_del(fmp, buddy);
		if ( offset > buddy )
			offset = buddy;
		++node.record.index;
	}

	fmp_list_add(fmp, node.record.index + 1, offset);
}

fmp_off_t
fmp_alloc_nothrow(fmempool *fmp, uint64_t size)
{
	fmp_head_t *head = &fmp->head;
	int flags = head->flags;
	if ( ! (flags & FMP_THROW) )
		return fmp_alloc(fmp, size);
	head->flags &= ~FMP_THROW;
	fmp_off_t offset = fmp_alloc(fmp, size);
	head->flags = flags;
	return offset;
}

// ** fmp_ read/write expand **

int
fmp_read(const fmempool *fmp, fmp_off_t offset, void *buf, uint64_t size)
{
//	int r = fseek(fmp->fp, offset, SEEK_SET);
	std::fstream *fs = fmp->fp;
	fs->seekg(offset);
	if ( fs->fail() )
		return 0;
	fs->read((char*)buf, size);
	return fs->good() ? 1 : 0;
}

int
fmp_write(const fmempool *fmp, fmp_off_t offset, const void *buf, uint64_t size)
{
//	int r = fseek(fmp->fp, offset, SEEK_SET);
	std::fstream *fs = fmp->fp;
	fs->seekp(offset);
	if ( fs->fail() )
		return 0;
	fs->write((char*)buf, size);
	return fs->good() ? 1 : 0;
}

int
fmp_head_read(fmempool *fmp)
{
//	fseek(fmp->fp, 0, SEEK_SET);
	std::fstream *fs = fmp->fp;
	fs->seekg(0);
	if ( fs->fail() )
		return 0;
	fs->read((char*)&fmp->head, sizeof(fmp_head_t));
	fs->read((char*)fmp->lists, sizeof(fmp_node_t)*fmp->head.nlists);
	return fs->good() ? 1 : 0;
}

int
fmp_head_write(const fmempool *fmp)
{
//	fseek(fmp->fp, 0, SEEK_SET);
	std::fstream *fs = fmp->fp;
	fs->seekp(0);
	if ( fs->fail() )
		return 0;
	fs->write((char*)&fmp->head, sizeof(fmp_head_t));
	fs->write((char*)fmp->lists, sizeof(fmp_node_t)*fmp->head.nlists);
	return fs->good() ? 1 : 0;
}

int
fmp_expand(fmempool *fmp, uint64_t size)
{
	std::fstream *fs = fmp->fp;

	// size = 2^n || size = 2^n - 2^m
	fmp_head_t *head = &fmp->head;

	fs->seekp(0, std::ios_base::end);
	if ( fs->fail() )
		return 0;
	int64_t tellp = fs->tellp();
	if ( tellp < 0 )
		return 0;
	uint64_t file_size = tellp;
	uint64_t need_size = head->end + size;

	if ( file_size < need_size ) {
		fs->seekp(file_size);
		if ( fs->fail() )
			return 0;
		uint64_t size = need_size - file_size; // local
		uint64_t n = size / FMP_BUFSIZ;
		uint64_t r = size % FMP_BUFSIZ;
		for (uint64_t i=0; i<n; ++i)
			fs->write((char*)fmp->buf, FMP_BUFSIZ);
		if ( r )
			fs->write((char*)fmp->buf, r);
	}

	uint64_t cap = head->end - head->begin;
	if ( ! cap )
		cap = size;
	while ( head->end < need_size ) {
		fmp_off_t offset = head->end;
		head->end += cap;

		fmp_node_t node;
		node.record.is_used = 1;
		node.record.index = fmp_lists_index(cap);
		node.record.size = 0;
		assert(fmp_write(fmp, offset, &node, RECORD_SIZE));
		fmp_free(fmp, offset + RECORD_SIZE);

		cap <<= 1;
	}

	return fs->good() ? 1 : 0;
}

int
fmp_reserve(fmempool *fmp, uint64_t size)
{
	// size = 2^n
	uint64_t max = fmp_max_block_size(fmp) + RECORD_SIZE;
	if ( size <= max )
		return 1;

	fmp_head_t *head = &fmp->head;
	uint64_t cap = head->end - head->begin;
	if ( ! cap )
		return fmp_expand(fmp, size);

	uint64_t new_cap = cap;
	do
		new_cap <<= 1;
	while ( (new_cap >> 1) < size );

	return fmp_expand(fmp, new_cap - cap);
}

void
fmp_memcpy(const fmempool *fmp, fmp_off_t dest, fmp_off_t src, uint64_t size)
{
	uint64_t n = size / FMP_BUFSIZ;
	uint64_t r = size % FMP_BUFSIZ;

	for (uint64_t i=0; i<n; ++i) {
		assert(fmp_read(fmp, src, fmp->buf, FMP_BUFSIZ));
		assert(fmp_write(fmp, dest, fmp->buf, FMP_BUFSIZ));
		dest += FMP_BUFSIZ;
		src += FMP_BUFSIZ;
	}
	if ( r ) {
		assert(fmp_read(fmp, src, fmp->buf, r));
		assert(fmp_write(fmp, dest, fmp->buf, r));
	}
}

// **  fmp_ max_block_size get_buddy  **

uint64_t
fmp_max_block_size(fmempool *fmp)
{
	fmp_node_t *lists = fmp->lists;
	for (int index=fmp->head.nlists-1; index>=0; --index) {
		fmp_node_t *list = &lists[index];
		if ( list->next )
			return (FMP_MIN_BLOCK << index) - RECORD_SIZE;
	}
	return 0;
}

fmp_off_t
fmp_get_buddy(fmempool *fmp, fmp_off_t offset, uint64_t index)
{
	fmp_head_t *head = &fmp->head;
	uint64_t capacity = FMP_MIN_BLOCK << index;

	fmp_off_t buddy_offset;
	if ( (offset - head->begin) & capacity )
		buddy_offset = offset - capacity;
	else {
		buddy_offset = offset + capacity;
		if ( buddy_offset + capacity > head->end )
			return OFF_NULL;
	}

	fmp_node_t buddy;
	assert(fmp_read(fmp, buddy_offset, &buddy, RECORD_SIZE));
	if ( buddy.record.is_used || buddy.record.index != index )
		return OFF_NULL;
	return buddy_offset;
}

// ** lists_index, node_init, node_add, node_del

int
fmp_lists_index(uint64_t size)
{
	return int64_highest_bit(size) - 4;
}

void
fmp_list_add(fmempool *fmp, fmp_off_t offset, fmp_off_t new_offset)
{
	assert(offset && new_offset);

	fmp_head_t *head = &fmp->head;
	fmp_node_t *lists = fmp->lists;
	fmp_node_t node, new_node;
	if ( offset <= (unsigned)head->nlists )
		node = lists[offset-1];
	else
		assert(0);

	new_node.record.is_used = 0;
	new_node.record.index = node.record.index;
	new_node.prev = offset;
	new_node.next = node.next;
	node.next = new_offset;

	if ( offset <= (unsigned)head->nlists )
		lists[offset-1] = node;
	else
		assert(0);

	if ( new_node.next ) {
		assert(fmp_read(fmp, new_node.next, &node, RECORD_SIZE));
		node.prev = new_offset;
		// prev is in record_t
		assert(fmp_write(fmp, new_node.next, &node, RECORD_SIZE));
	}

	assert(fmp_write(fmp, new_offset, &new_node, NODE_SIZE));
}

void
fmp_list_del(fmempool *fmp, fmp_off_t offset)
{
	assert(offset);

	fmp_head_t *head = &fmp->head;
	fmp_node_t *lists = fmp->lists;
	fmp_node_t prev_node, node;
	assert(fmp_read(fmp, offset, &node, NODE_SIZE));
	if ( node.prev <= (unsigned)head->nlists )
		prev_node = lists[node.prev-1];
	else
		assert(fmp_read(fmp, node.prev, &prev_node, NODE_SIZE));

	prev_node.next = node.next;
	if ( node.next ) {
		fmp_node_t next_node;
		assert(fmp_read(fmp, node.next, &next_node, RECORD_SIZE));
		next_node.prev = node.prev;
		// prev is in record_t
		assert(fmp_write(fmp, node.next, &next_node, RECORD_SIZE));
	}

	if ( node.prev <= (unsigned)head->nlists )
		lists[node.prev-1] = prev_node;
	else
		assert(fmp_write(fmp, node.prev, &prev_node, NODE_SIZE));
}

// ** fmp_ check, check_list, throw_ofm, print **

void
fmp_check(const fmempool *fmp)
{
	assert(fmp);
	const fmp_head_t *head = &fmp->head;
	const fmp_node_t *lists = fmp->lists;
	uint64_t capacity = head->end - head->begin;
	assert(0 < head->nlists && head->nlists <= FMP_NLISTS);
	assert(head->nalloc < capacity);
	assert(head->nfree <= capacity);
	assert(head->nalloc + head->nfree <= capacity);
	for (int index=0; index<head->nlists; ++index) {
		assert(lists[index].record.index == index);
		fmp_check_list(fmp, &lists[index]);
	}

	fmp_node_t node;
	fmp_off_t offset = head->begin;
	while ( offset < head->end ) {
		assert(fmp_read(fmp, offset, &node, NODE_SIZE));
		uint64_t cap = FMP_MIN_BLOCK << node.record.index;
		assert( node.record.index < head->nlists );
		if ( node.record.is_used )
			assert(node.record.size + RECORD_SIZE <= cap);
		offset += cap;
	}
}

void
fmp_check_list(const fmempool *fmp, const fmp_node_t *list)
{
	fmp_node_t node, next_node;
	fmp_off_t node_off = list->record.index + 1;

	node = *list;
	while ( node.next ) {
		assert(fmp_read(fmp, node.next, &next_node, NODE_SIZE));
		assert(next_node.prev == node_off);
		assert( ! next_node.record.is_used );
		node_off = node.next;
		node = next_node;
	}
}

void
fmp_throw_ofm(const char *file, uint64_t line, const char *func, const char *msg, uint64_t size)
{
	std::cerr<<"fmempool: "<<file<<":"<<line<<": "<<func<<"[size = "<<size<<"]: "<<msg<<std::endl;
	abort();
}

void
fmp_print(const fmempool *fmp, std::ostream& out)
{
	assert(fmp);
	const fmp_head_t *head = &fmp->head;
	uint64_t capacity = head->end - head->begin;
	out<<"[ "<<capacity<<" total, "<<head->nfree<<" free, "<<head->nalloc<<" used ]\n";

	if ( ! capacity ) {
		out<<"||\n"<<std::endl;
		return;
	}

	fmp_node_t node;
	fmp_off_t offset = head->begin;
	out<<"|";
	while ( offset < head->end ) {
		assert(fmp_read(fmp, offset, &node, NODE_SIZE));
		uint64_t cap = FMP_MIN_BLOCK << node.record.index;
		if ( ! node.record.is_used )
			out<<cap<<"|";
		else
			out<<node.record.size<<"/"<<cap<<"|";
		offset += cap;
	}
	out<<'\n'<<std::endl;
}

// ** min2pow, int64_highest_bit **

int64_t
min2pow(int64_t n) { // 不小于n的 最小2的幂
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

} // namespace c

fmempool::fmempool(std::fstream *fs, uint64_t size, int flags)
{
	fmp = akm::c::fmp_init(fs, size, flags);
	assert(fmp);
}

fmempool::~fmempool()
{
	akm::c::fmp_close(fmp);
}

fmp_off_t
fmempool::alloc(uint64_t size)
{
	return akm::c::fmp_alloc(fmp, size);
}

fmp_off_t
fmempool::realloc(fmp_off_t offset, uint64_t size)
{
	return akm::c::fmp_realloc(fmp, offset, size);
}

void
fmempool::free(fmp_off_t offset)
{
	akm::c::fmp_free(fmp, offset);
}

bool
fmempool::read(fmp_off_t offset, void *buf, uint64_t size) const
{
	return akm::c::fmp_read(fmp, offset, buf, size);
}

bool
fmempool::write(fmp_off_t offset, const void *buf, uint64_t size) const
{
	return akm::c::fmp_write(fmp, offset, buf, size);
}

bool
fmempool::flush() const
{
	if ( ! akm::c::fmp_head_write(fmp) )
		return false;
	std::fstream *fs = fmp->fp;
	fs->flush();
	return fs->good();
}

void
fmempool::memcpy(fmp_off_t dest, fmp_off_t src, uint64_t size) const
{
	akm::c::fmp_memcpy(fmp, dest, src, size);
}

void
fmempool::check() const
{
	akm::c::fmp_check(fmp);
}

std::ostream&
operator<< (std::ostream& out, const fmempool& fmp)
{
	akm::c::fmp_print(fmp.fmp, out);
	return out;
}

} // namespace akm

