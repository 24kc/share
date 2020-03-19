#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "fmempool.h"
using namespace std;

#define N  (2000)
#define M  ( ( (rand()%11) << (rand()%11) ) + 1 )

const char *fname = "0";

int main()
{
	fstream f(fname, std::ios::binary | std::ios::in | std::ios::out);
	if ( ! f.is_open() ) {
		f.open(fname, std::ios::binary | std::ios::out);
		f.close();
		f.open(fname, std::ios::binary | std::ios::in | std::ios::out);
	}
	if ( ! f.is_open() ) {
		cout<<"fail to open file "<<fname<<endl;
		return -1;
	}

	srand(time(NULL));

	fmempool *fmp = fmp_init(&f, N, FMP_CREAT | FMP_THROW);
	fmp_print(fmp);
	fmp_check(fmp);

	fmp_off_t a[N];

	for (int i=0; i<N; ++i) {
		a[i] = fmp_alloc(fmp, rand()%M);
	}

	fmp_print(fmp);
	fmp_check(fmp);

	for (int i=0; i<N/2; ++i) {
		int index = rand()%N;
		if ( ! a[index] )
			continue;
		fmp_free(fmp, a[index]);
		a[index] = OFF_NULL;
		fmp_check(fmp);
	}

	fmp_print(fmp);
	fmp_check(fmp);

	for (int i=0; i<N; ++i) {
		if ( ! a[i] )
			continue;
		int new_size = rand()%(M<<1);
	//	printf("%d => %d\n", ((fmp_node_t*)a[i]-1)->size, new_size);
		a[i] = fmp_realloc(fmp, a[i], new_size);
//		fmp_check(fmp);
	}

	fmp_print(fmp);
	fmp_check(fmp);

	for (int i=0; i<N; ++i) {
		if ( ! a[i] )
			continue;
		fmp_free(fmp, a[i]);
		fmp_check(fmp);
	}

	fmp_print(fmp);
	fmp_check(fmp);
}

