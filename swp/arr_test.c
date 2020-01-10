#include <stdio.h>
#define type int
#include "array.h"

int main()
{
	array a = new_array(0);

	int i;
	while ( scanf("%d", &i) == 1 )
		array_add(&a, i);

	for (i=0; i<a.length; ++i)
		printf("%d ", a.data[i]);

	puts("");
}

