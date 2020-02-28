#ifndef _MY_NEW_H_
#define _MY_NEW_H_

#include "mempool.h"

#ifndef _MY_NEW_C_
extern akm::mempool *_mp;
#else
akm::mempool *_mp = NULL;
#endif

void* operator new (size_t);
void* operator new[] (size_t);
void operator delete (void*) noexcept;
void operator delete[] (void*) noexcept;

#endif

