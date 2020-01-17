#include <stdio.h>
#define type int*
#include "array.h"

int main() 
{
	int i;
	int *p = (int*)malloc(sizeof(int));

	array a1 = new_array(0);
	array a2 = new_array(0);

	for (i=0; i<10; ++i) {
		array_add(&a1, NULL);
		array_add(&a2, NULL);
	}

	array_add(&a1, p);
	array_add(&a2, p);

	free(a1.data[a1.length-1]);

	for (i=0; i<a2.length; ++i)
		printf("%p ", a2.data[i]);
	puts("");

	return 24-'k';
}
