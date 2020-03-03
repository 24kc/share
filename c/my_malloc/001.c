#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mempool.h"

#define N  (2000)
#define M  ( ( (rand()%11) << (rand()%11) ) + 1 )

int main()
{
	srand(time(NULL));

	static char mem[999999*3];
	mempool *mp = mp_init(mem, sizeof(mem), MP_THROW);
	mp_print(mp);
	mp_check(mp);

	int *a[N];

	for (int i=0; i<N; ++i) {
		a[i] = (int*)mp_alloc(mp, rand()%M);
	}

	mp_print(mp);
	mp_check(mp);

	for (int i=0; i<N/2; ++i) {
		int index = rand()%N;
		if ( ! a[index] )
			continue;
		mp_free(mp, a[index]);
		a[index] = NULL;
		mp_check(mp);
	}

	mp_print(mp);
	mp_check(mp);

	for (int i=0; i<N; ++i) {
		if ( ! a[i] )
			continue;
		int new_size = rand()%(M<<1);
	//	printf("%d => %d\n", ((mp_node_t*)a[i]-1)->size, new_size);
		a[i] = (int*)mp_realloc(mp, a[i], new_size);
		mp_check(mp);
	}

	mp_print(mp);
	mp_check(mp);

	for (int i=0; i<N; ++i) {
		if ( ! a[i] )
			continue;
		mp_free(mp, a[i]);
		mp_check(mp);
	}

	mp_print(mp);
	mp_check(mp);
}

