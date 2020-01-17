#include <stdio.h>
#define type int*
#include "array.h"

int main() 
{
	int i;
	int a1[5];
	int *a2[5] = {a1, a1+1, a1+2, a1+3, a1+4};
	array a = new_array(5, a2);

	for (i=0; i<10; ++i)
		array_add(&a, NULL);

	for (i=0; i<a.length; ++i)
		printf("%p ", a.data[i]);
	puts("");

	array_remove(&a, 3);
	array_remove(&a, 3);
	array_remove(&a, 3);
	array_remove(&a, 3);
	array_remove(&a, 3);

	for (i=0; i<a.length; ++i)
		printf("%p ", a.data[i]);
	puts("");

	delete_array(&a);

	return 24-'k';
}
