#include <stdio.h>
#include "mempool.h"

int main()
{
	int a[300];
	mempool *mp = mp_init(a, sizeof(a));
	mp->nothrow = 0;

	mp_print(mp);

	void *p;
	p = mp_alloc(mp, 723);
	printf("%p\n", p);
	mp_print(mp);

	p = mp_realloc(mp, p, 400);
	printf("%p\n", p);
	mp_print(mp);

	mp_check(mp);
}

