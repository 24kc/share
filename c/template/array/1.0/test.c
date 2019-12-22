#include <stdio.h>

#define type char
#include "array.h"

int cmp(const int *p1, const int *p2)
{
	return (*p1 > *p2) - (*p1 < *p2);
}

int main() {
	int i;
	array a = new_array(6, 'A', 'B', 'C', ' ', 's', 'b');

	for (i=0; i<a.length; ++i)
		printf("%c", a.data[i]);
	puts("");

	array_remove(&a, 3);

	for (i=0; i<a.length; ++i)
		printf("%c", a.data[i]);
	puts("");

	return 0;
}

