#include <stdio.h>
#include "mempool.h"

int main()
{
	int a[999];
	mempool *mp = mp_init(a, sizeof(a));
	mp_print(mp);

	int n;
	while ( (mp = mp_max_block(mp, &n, 1)) ) {
		mp = mp_init(mp, n);
		if ( ! mp )
			break;
		mp_print(mp);
	}

}

