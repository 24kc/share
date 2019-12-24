#ifndef _BASIC_MEMPOOL_H_
#define _BASIC_MEMPOOL_H_

#include <iostream>

#define __t(T)		template <typename T>
#define OFF_NULL	(0)

namespace akm{
using istream = std::istream;
using ostream = std::ostream;

__t(T)
class basic_mempool{

	struct mp_size_t { T t; };

  public:
	basic_mempool(); // 不构建内存池, 需要使用init(count)来构建内存池
	basic_mempool(int count); // 构建内存池, 初始容量为count
	~basic_mempool();

	int alloc(); // 向内存池租用内存, 返回偏移, 可通过getptr()获取地址
	void free(int off); // 把内存还给内存池
	void* getptr(int off); // 通过alloc()返回的偏移获取[临时]内存地址

	bool init(int count = 0); // 构建内存池,初始容量为count个T大小; 若参数不填,则count为capacity();
	bool resize(int count); // 调整内存池容量大小,缩小内存池时会清空内存租用记录、重置内存池; count=0时同destroy();
	void destroy(); // 释放内存, 销毁内存池
	void reset(); // 清空内存租用记录, 重置内存池

	int size(); // 返回内存池中已租用内存块数量
	int capacity(); // 返回内存池容量(以T大小为单位)
	bool empty(); // 是否有已租用的内存

	void write(ostream& out); // 把内存池数据写入out, 还需out.write(this)
	void read(istream& in); // 需要in.read(this), init(). 再从文件read数据到内存池

  private:
	mp_size_t *base; // (base+1)指向存储元素的内存, (base+0)为非法内存
	int mp_size; // 已租用内存块数量
	int mp_capacity; // 总容量

	int *index; // 存储租用后返还的内存块的偏移
	int index_size; // index数组中有效数据的数量
};

__t(T)
basic_mempool<T>::basic_mempool()
{
	base = NULL;
	mp_size = 0;
	mp_capacity = 0;
	index = NULL;
	index_size = 0;
}

__t(T)
basic_mempool<T>::basic_mempool(int count)
{
	init(count);
}

__t(T)
basic_mempool<T>::~basic_mempool()
{
	destroy();
}

__t(T)
int
basic_mempool<T>::alloc()
{
	return index_size ? index[--index_size] :
		(
			mp_size == mp_capacity
			? (resize(mp_capacity<<1) ? alloc() : OFF_NULL)
			: ++mp_size
		) ;
}

__t(T)
void
basic_mempool<T>::free(int off)
{
	index[index_size++] = off;
}

__t(T)
void*
basic_mempool<T>::getptr(int off)
{
	return (void*)(base + off);
}

__t(T)
bool
basic_mempool<T>::init(int count)
{
	if ( ! count ) {
		if ( ! mp_capacity )
			return 0;
		count = mp_capacity;
	} else {
		mp_size = 0;
		mp_capacity = count;
		index_size = 0;
	}

	index = (int*)malloc(sizeof(int) * count);
	if ( ! index ) {
		base = NULL;
		mp_capacity = 0;
		return 0;
	}

	base = (mp_size_t*)malloc(sizeof(mp_size_t) * count);
	if ( ! base ) {
		::free(index);
		index = NULL;
		mp_capacity = 0;
		return 0;
	}
	--base; // base+0 for OFF_NULL

	return 1;
}

__t(T)
bool
basic_mempool<T>::resize(int count)
{
	if ( count < mp_capacity )
		reset();

	void *p = realloc(index, sizeof(int) * count);
	if ( ! p )
		return 0;
	index = (int*)p;

	p = realloc(base+1, sizeof(mp_size_t) * count); // base+0 for OFF_NULL
	if ( ! p )
		return 0;
	base = (mp_size_t*)p - 1;

	mp_capacity = count;

	return 1;
}

__t(T)
void
basic_mempool<T>::destroy()
{
	if ( base ) {
		::free(base+1);
		base=NULL;
	}
	if ( index ) {
		::free(index);
		index=NULL;
	}
	mp_size = 0;
	mp_capacity = 0;
	index_size = 0;
}

__t(T)
inline void
basic_mempool<T>::reset()
{
	mp_size = 0;
	index_size = 0;
}

__t(T)
int
basic_mempool<T>::size()
{
	return mp_size - index_size;
}

__t(T)
int
basic_mempool<T>::capacity()
{
	return mp_capacity;
}

__t(T)
bool
basic_mempool<T>::empty()
{
	return mp_size == index_size;
}

__t(T)
void
basic_mempool<T>::write(ostream& out)
{
	if ( ! mp_capacity )
		return;

	out.write((char*)(base+1), sizeof(mp_size_t) * mp_size);
	out.write((char*)index, sizeof(int) * index_size);
}

__t(T)
void
basic_mempool<T>::read(istream& in)
{
	if ( ! mp_capacity )
		return;

	in.read((char*)(base+1), sizeof(mp_size_t) * mp_size);
	in.read((char*)index, sizeof(int) * index_size);
}

} // namespace akm;

#undef __t

#endif //_BASIC_MEMPOOL_H_

