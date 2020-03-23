#ifndef _FIRST_FIT_H_
#define _FIRST_FIT_H_

#include <stdint.h>
#include <stddef.h>

#define MP_THROW  (0x8) // 内存不足时终止程序(使用abort())

// mempool 不使用额外内存, 内部采用偏移, 支持fwrite/fread复现
typedef struct {
	int32_t flags;
	uint64_t capacity;
	uint64_t nalloc;
	uint64_t nfree;
	uint64_t first_free; // 第一个空闲块偏移
} mempool;

#ifdef __cplusplus
extern "C" {
#endif

/* std: */
mempool* mp_init (void *mem, size_t size, int flags);
// 在大小为size的内存块mem上建立内存池
	// flags:
		// MP_THROW 指定在内存不足时抛出异常(使用abort())
	// 成功: 返回的地址与mem相同
	// 失败: 返回NULL (size过小时会失败)
void* mp_alloc (mempool *mp, size_t size);
void* mp_realloc (mempool *mp, void *mem, size_t size);
void mp_free (mempool *mp, void *mem);
// 类似 malloc, realloc, free

/* for 套娃: */
size_t mp_max_block_size (mempool *mp);
// 返回最大可用内存块的大小, 内存已用完则为0

/* for debug: */
void mp_check (mempool*);
// 初步检查内存池数据是否有误, 出错直接抛出异常
void mp_print (mempool*);
// 输出内存池中内存分布信息

#ifdef __cplusplus
}
#endif

#endif // _FIRST_FIT_H_

