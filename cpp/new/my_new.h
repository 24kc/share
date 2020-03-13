#ifndef _MY_NEW_H_
#define _MY_NEW_H_

#include "mempool.h"

namespace akm {
#ifndef _MY_NEW_CPP_
extern mempool *mp;
#else
mempool *mp = NULL;
#endif
} // namespace akm

void* operator new (size_t);
void* operator new[] (size_t);
void operator delete (void*) noexcept;
void operator delete[] (void*) noexcept;

#endif

