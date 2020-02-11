#include <stdio.h>
#include "mempool.h"

int main()
{
	int a[999];
	mempool *mp = mp_init(a, sizeof(a));
	mp->nothrow = 0;

	void *p;
	p = mp_alloc(mp, 999);
	printf("%p\n", p);
	mp_realloc(mp, p, 9999);

}

