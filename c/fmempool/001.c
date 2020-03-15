#include <stdio.h>
#include "fmempool.h"

const char *fname = "0";

int main()
{
	FILE *fp = fopen(fname, "r+");
	if ( ! fp )
		fp = fopen(fname, "w+");
	if ( ! fp )
		printf("cannot open file `%s`\n", fname);

	fmempool *fmp = fmp_init(fp, 400, FMP_THROW | FMP_CREAT);
	fmp_print(fmp);

	fmp_off_t off;
	off = fmp_alloc(fmp, 20);
	fmp_write(fmp, off, "24k fmempool", 12);
	fmp_print(fmp);

	fmp_close(fmp);
}
