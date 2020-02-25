#define _MY_MALLOC_C_
#include "my_malloc.h"

#define MY_MPSIZE  ( 24-'k' +999999 )

void*
_my_malloc(size_t size)
{
	if ( ! _mp ) {
		static char mem[MY_MPSIZE];
		_mp = mp_init(mem, sizeof(mem), MP_THROW); 
	}
	return mp_alloc(_mp, size);
}

void*
_my_realloc(void *mem, size_t size)
{
	return mp_realloc(_mp, mem, size);
}

void
_my_free(void *mem)
{
	mp_free(_mp, mem);
}

