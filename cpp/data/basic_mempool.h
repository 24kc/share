#ifndef _BASIC_MEMPOOL_H_
#define _BASIC_MEMPOOL_H_

#include <stdio.h>
#include <stdlib.h>

#define __t(T)		template <typename T>
#define OFF_NULL	(0)

namespace akm{

__t(T)
class basic_mempool{

	class mp_size_t { T t; };

  public:
	basic_mempool(); // 不构建内存池, 需要使用init(count)来构建内存池
	basic_mempool(int count); // 构建内存池, 初始容量为count
	~basic_mempool();

	int alloc(); // 向内存池租用内存, 返回偏移, 可通过getptr()获取地址
	void free(int off); // 把内存还给内存池
	void* getptr(int off); // 通过alloc()返回的偏移获取[临时]内存地址

	int init(int count = 0); // 构建内存池,初始容量为count个T大小; 若参数不填,则count为capacity();
	int resize(int count); // 调整内存池大小,缩小内存池时会清空内存分配记录、重置内存池; count=0时同destroy();
	void destroy(); // 释放内存, 销毁内存池
	void reset(); // 清空内存分配记录, 重置内存池

	int size(); // 返回内存池中已租用内存块数量
	int capacity(); // 返回内存池容量(以T大小为单位)
	int empty(); // 是否有已租用的内存

	void write_file(FILE *fp);
	void read_file(FILE *fp);

  private:
	mp_size_t *base; // (base+1)指向存储元素的内存, (base+0)为非法内存
	int mp_size; // 已租用内存块数量
	int mp_capacity; // 总容量

	int *index; // 存储租用后返还的内存块的偏移
	int index_size; // index数组中有效数据的数量
};

} //namespace akm

#undef __t

#endif //_BASIC_MEMPOOL_H_

