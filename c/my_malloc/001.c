#include <stdio.h>
#include <malloc.h>
#define MY_MP_THROW  1
#include "my_malloc.h"
#define type int
#include "array.h"

#define mp  __my_malloc_mp__

int main()
{
	int last_cap = 0;
	array a = new_array(0);

	for (int i=0; ; ++i) {
		array_add(&a, i);
		if ( a.capacity != last_cap ) {
			last_cap = a.capacity;
			mp_print(mp);
		}
	}
}

