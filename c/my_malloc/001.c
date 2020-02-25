#include <stdio.h>
#include "my_malloc.h"
#define type int
#include "array.h"

int main()
{
	int last_cap = 0;
	array a = new_array(0);

	for (int i=0; i<10000; ++i) {
		array_add(&a, i);
		if ( a.capacity != last_cap ) {
			last_cap = a.capacity;
			mp_print(_mp);
		}
	}
}

