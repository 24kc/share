#include <stdio.h>

#define type char
#include "array.h"

int cmp(const char *p1, const char *p2)
{
	return (*p1 > *p2) - (*p1 < *p2);
}

int main() {
	int i;
	char s[10] = {"ABC sb"};
	array a = new_array(sizeof(s), s);

	for (i=0; i<a.length; ++i)
		printf("%c", a.data[i]);
	puts("");

	array_remove_many(&a, 1, 3);

	for (i=0; i<a.length; ++i)
		printf("%c", a.data[i]);
	puts("");

	delete_array(&a);

	return 0;
}

