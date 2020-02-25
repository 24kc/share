#ifndef _MY_MALLOC_H_
#define _MY_MALLOC_H_

#ifndef _MEMPOOL_H_
#include "mempool.h"
#endif

#include <stdlib.h>

#ifndef _MY_MALLOC_C_
extern mempool *_mp;
#else
mempool *_mp = NULL;
#endif

void* _my_malloc(size_t size);
void* _my_realloc(void *mem, size_t size);
void _my_free(void *mem);

#define malloc(size)  _my_malloc(size)
#define realloc(mem, size)  _my_realloc(mem, size)
#define free(mem)  _my_free(mem)

#endif

