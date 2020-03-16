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

	fmempool *fmp = fmp_init(fp, 400, FMP_CREAT|FMP_THROW);
	fmp_print(fmp);

	fmp_off_t off;
	off = fmp_alloc(fmp, 40);
	fmp_write(fmp, off, "24k fmempool", 12);
	fmp_check(fmp);
	fmp_print(fmp);

	off = fmp_realloc(fmp, off, 500);
	fmp_write(fmp, off, "24k fmempool", 2);
	fmp_check(fmp);
	fmp_print(fmp);

	off = fmp_realloc(fmp, off, 0);
	fmp_write(fmp, off, "24k fmempool", 2);
	fmp_check(fmp);
	fmp_print(fmp);

	fmp_check(fmp);
	fmp_close(fmp);
}
