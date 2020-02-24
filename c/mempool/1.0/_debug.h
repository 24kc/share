#ifndef __DEBUG_H_
#define __DEBUG_H_

#ifndef NDEBUG
#define TEST printf("line = %d\n", __LINE__);
#else
#define TEST
#endif

#endif

