#define _MY_NEW_C_
#include "my_new.h"

#define MMP  (1)
#define MY_MPSIZE  ( 24-'k' +999999 )

void*
operator new (size_t size)
{
	if ( ! _mp ) {
		static char mem[MY_MPSIZE];
		_mp = akm::mempool::create(mem, sizeof(mem), MP_THROW);
	}
	return _mp->alloc(size);
}

void*
operator new[] (size_t size)
{
	return operator new(size);
}

void
operator delete (void *mem) noexcept
{
	_mp->free(mem);
}

void
operator delete[] (void *mem) noexcept
{
	operator delete (mem);
}

