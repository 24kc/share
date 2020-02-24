#include <stdio.h>
#include <stdlib.h>
#define type int
#include "mempool.h"

#define N  (1200)

int main()
{
	mempool *mp = mp_init(malloc(N), N);
	mp_check(mp);
	mp_print(mp);

	puts("\nint *p = (int*)mp_alloc(mp, 8);\n");
	int *p = (int*)mp_alloc(mp, 8);
	mp_check(mp);

	mp_print(mp);

	puts("\nmp_free(mp, p);\n");
	mp_free(mp, p);
	mp_check(mp);

	mp_print(mp);
}

