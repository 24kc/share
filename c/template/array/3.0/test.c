#include <stdio.h>

#define type char
#include "array.h"

int cmp(const char *p1, const char *p2)
{
	return (*p1 > *p2) - (*p1 < *p2);
}

int main() {
	int i;
	char s[] = {"ABC is just a sb"};
	char s2[] = {"sb"};
	array a = new_array(sizeof(s)-1, s);

	for (i=0; i<a.length; ++i)
		printf("%c", a.data[i]);
	puts("");

	array_sort(&a, cmp);
	array_remove_many(&a, 0, 4);
	array_insert_many(&a, 3, sizeof(s2), s2);

	printf("%s", a.data);
	puts("");

	delete_array(&a);

	return 0;
}

