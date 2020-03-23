#include <stdio.h>
#include "first_fit.h"

int main()
{
	char a[1200];
	mempool *mp = mp_init(a, sizeof(a), MP_THROW);
	mp_print(mp);

	int *p = (int*)mp_alloc(mp, 4);
	*p = 24-'k';
	mp_print(mp);

	p = (int*)mp_realloc(mp, p, 40);
	p[9] = 24+'k';
	mp_print(mp);

	mp_free(mp, p);
	mp_print(mp);
}
