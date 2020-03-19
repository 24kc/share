#include <iostream>
#include <fstream>
#include "fmempool.h"
using namespace std;

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

	fmempool *fmp = fmp_init(&f, 1000, FMP_CREAT | FMP_THROW);
	fmp_print(fmp);

	fmp_off_t off = fmp_alloc(fmp, 120);
	fmp_write(fmp, off, "24k fmempool", 12);
	fmp_print(fmp);

	off = fmp_realloc(fmp, off, 4000000);
	fmp_print(fmp);

	fmp_free(fmp, off);
	fmp_print(fmp);

	fmp_close(fmp);
}
