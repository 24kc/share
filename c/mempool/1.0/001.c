#include <stdio.h>
#include <stdlib.h>
#define type int
#include "mempool.h"
#include <string.h>

#define N  (1200)

int main()
{
	mempool *mp = mp_init(malloc(N), N);
	mp_check(mp);
	mp_print(mp);

	void *p, *p1;

	p = mp_alloc(mp, 400);
	p1 = mp_alloc(mp, 416);
	mp_check(mp);
	mp_print(mp);

	p = mp_realloc(mp, p, 14);
	mp_check(mp);
	mp_print(mp);

	int n;
	mp_free(mp, p1);
	p = mp_max_block(mp, &n, 1);
	mp_check(mp);
	mp_print(mp);
	printf("%p %d\n", p, n);

	mempool *mp1 = mp_init(p, n);
	mp_check(mp1);
	mp_print(mp1);
}

