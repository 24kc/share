#ifndef _FMEMPOOL_H_
#define _FMEMPOOL_H_

#include <stdio.h>
#include <stdint.h>

#define FMP_CREAT  (0x4) // 创建文件内存池
#define FMP_THROW  (0x8) // 内存不足时抛出异常(使用abort())
#define FMP_NOADD  (0x10) // 内存不足时不自动扩展内存

typedef uint64_t fmp_off_t;

typedef struct {
	int32_t flags;
	int32_t nlists; // list num
	uint64_t nalloc; // alloc size
	uint64_t nfree; // free size, nfree <= capacity
	fmp_off_t begin, end;
} fmp_head_t;
// fmempool head data

// 记录内存分配信息
typedef struct fmp_record_t {
	uint64_t size:48; // 提供的内存大小
	uint64_t index:8; // lists[index], (index:6)
	uint64_t :7;
	uint64_t is_used:1; // 是否已分配
} fmp_record_t;

// fmempool record node
typedef struct {
	union {
		uint64_t prev:48;
		fmp_record_t record;
	};
	uint64_t next;
} fmp_node_t;
// [1, FMP_NLISTS] -> lists

#define FMP_NLISTS  (44)

typedef struct {
	FILE *fp;
	fmp_head_t head;
	fmp_node_t lists[FMP_NLISTS];
	void *buf;
} fmempool;

#ifdef __cplusplus
extern "C" {
#endif

fmempool* fmp_init (FILE *fp, uint64_t size, int flags);
// 文件内存池初始化, 内存池大小不小于size
	// flags:
		// FMP_CREAT 创建内存池
		// FMP_THROW 指定在内存不足时抛出异常(使用abort())
	// 成功: 返回非0
	// 失败: 返回0
void fmp_close (fmempool *fmp);
// 把缓冲区数据写入磁盘, fmp无效化, 不关闭文件

fmp_off_t fmp_alloc (fmempool *fmp, uint64_t size);
//fmp_off_t fmp_realloc (fmempool *fmp, fmp_off_t offset, uint64_t size);
void fmp_free (fmempool *fmp, fmp_off_t offset);
// 类似 malloc, realloc, free

int fmp_read(fmempool *fmp, fmp_off_t offset, void *buf, uint64_t size);
int fmp_write(fmempool *fmp, fmp_off_t offset, const void *buf, uint64_t size);
// 读写数据, 成功返回非0, 失败返回0

/* for debug: */
void fmp_check (fmempool *fmp);
// 初步检查内存池数据是否有误, 出错直接抛出异常
void fmp_print (fmempool *fmp);
// 输出内存池中内存分布信息

#ifdef __cplusplus
}
#endif

#endif
