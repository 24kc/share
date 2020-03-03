#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#define MP_MIN_BLOCK  (32)
// 2^n >= 32

#define MP_THROW  (0x10)

typedef struct mp_node_t{
	int size; // Application size
	int capacity; // Actual capacity
	struct mp_node_t *prev;
	struct mp_node_t *next;
} mp_node_t; // mempool node

typedef struct {
	mp_node_t *list; // some lists store the released nodes
	int list_num;
	int flag;
	void *begin, *end; // mempool
} mempool;

#ifdef __cplusplus
extern "C" {
#endif

mempool* mp_init (void *mem, int size, int flag);
// 在大小为size的内存块mem上建立内存池
	// flag: 内存不足时若flag为0则返回NULL, 为MP_THROW则抛出异常
	// 成功: 返回的地址与mem相同
	// 失败: 返回NULL (size过小时会失败)

void* mp_alloc (mempool *mp, int size);
void* mp_realloc (mempool *mp, void *mem, int size);
void mp_free (mempool *mp, void *mem);
// 类似 malloc, realloc, free

int mp_capacity (mempool *mp);
// 返回内存池容量

void* mp_max_block (mempool *mp, int *p_size, int flag);
// 申请最大内存块
// 返回内存池当前最大可用内存块, 并设置*p_size为内存块大小
// 若flag为0, 则不分配内存, 仅通过*p_size返回最大内存块大小
// 若内存已满则设置*p_size为0

void mp_check (mempool*);
// 初步检查内存池数据是否有误, 出错直接抛出异常

void* mp_alloc_nothrow (mempool *mp, int size);
void* mp_realloc_nothrow (mempool *mp, void *mem, int size);
// 类似mp_alloc, mp_realloc
// 不论内存池是否设置了MP_THROW标志, 始终不抛出异常

void mp_print(mempool*);
// 输出内存池中分配信息

#ifdef __cplusplus
}
#endif

#endif
