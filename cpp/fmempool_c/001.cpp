#include <iostream>
#include <fstream>
#include "fmempool.h"
using namespace std;
using namespace akm;

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

	fmempool fmp(&f, 0, FMP_CREAT | FMP_THROW);
	cout<<fmp;

	auto off = fmp.alloc(12);
	fmp.write(off, "24k fmempool", 12);
	cout<<fmp;

	fmp.free(off);
	cout<<fmp;
}
