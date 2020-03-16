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

	fmempool *fmp = fmp_init(fp, 1000, FMP_CREAT | FMP_THROW);
	fmp_print(fmp);

	fmp_off_t off = fmp_alloc(fmp, 12);
	fmp_write(fmp, off, "24k fmempool", 12);
	fmp_print(fmp);

	off = fmp_realloc(fmp, off, 400);
	fmp_print(fmp);

	fmp_free(fmp, off);
	fmp_print(fmp);

	fmp_close(fmp);
}
