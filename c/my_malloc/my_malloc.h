#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_

#ifndef _MEMPOOL_H_
#include "mempool.h"
#endif

#ifndef MY_MPSIZE
#define MY_MPSIZE  ( 24-'k' +999999 )
#endif

#ifndef MY_MP_THROW
#define MY_MP_THROW  (0)
#endif

#define _mp __my_malloc_mp__

static mempool *_mp = NULL;

static
void*
_my_malloc(size_t size)
{
	if ( ! _mp ) {
		static char mem[MY_MPSIZE];
		_mp = mp_init(mem, sizeof(mem)); 
		_mp->nothrow = ! MY_MP_THROW;
	}
	return mp_alloc(_mp, size);
}

static
void*
_my_realloc(void *p, size_t size)
{
	return mp_realloc(_mp, p, size);
}

static
void
_my_free(void *p)
{
	mp_free(_mp, p);
}

#undef _mp

#define malloc(size)  _my_malloc(size)
#define realloc(p, size)  _my_realloc(p, size)
#define free(p)  _my_free(p)

#endif
