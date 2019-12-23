#include "basic_mempool.h"

#define __t(T)		template <typename T>
#define OFF_NULL	(0)

namespace akm{

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
		(mp_size==mp_capacity
		? (resize(mp_capacity*2) ? alloc() : OFF_NULL)
		: ++mp_size);
}

__t(T)
void*
basic_mempool<T>::getptr(int off)
{
	return (void*)(base + off);
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
}

__t(T)
void
basic_mempool<T>::writefile(FILE *fp)
{
	if (!mp_capacity)
		return;
	fwrite(base+1, sizeof(T), mp_size, fp);
	fwrite(index, sizeof(int), index_size, fp);
}

__t(T)
void
basic_mempool<T>::readfile(FILE *fp)
{
	if (!mp_capacity)
		return;
	fread(base+1, sizeof(T), mp_size, fp);
	fread(index, sizeof(int), index_size, fp);
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
basic_mempool<T>::resize(int count)
{
	void *p;
	if (count < mp_capacity)
		reset();
	p = realloc(index, sizeof(int)*count);
	if (!p)
		return false;
	index = (int*)p;
	p = realloc(base+1, sizeof(T)*count); // base+0 for OFF_NULL
	if (!p)
		return false;
	base = (mp_size_t*)p - 1;
	mp_capacity = count;
	return true;
}

__t(T)
int
basic_mempool<T>::init(int count)
{
	if (!count) {
		if (!mp_capacity)
			return false;
		count = mp_capacity;
	} else {
		mp_size = 0;
		mp_capacity = count;
		index_size = 0;
	}
	index = (int*)malloc(sizeof(int)*count);
	if (!index) {
		base = NULL;
		mp_capacity = 0;
		return false;
	}
	base = (mp_size_t*)malloc(sizeof(T)*count);
	if (!base) {
		::free(index);
		index = NULL;
		mp_capacity = 0;
		return false;
	}
	--base; // base+0 for OFF_NULL
	return true;
}
__t(T)
inline void
basic_mempool<T>::free(int off)
{
	index[index_size++] = off;
}

} // namespace akm;

