#include <stdio.h>

#define type int
#include "list.h"

int cmp(const int *p1, const int *p2)
{
	return (*p1 > *p2) - (*p1 < *p2);
}

int main() {
	int a[10] = {85, 60, 90, 89, 9, 49, 92, 76, 95, 55};
	int i, *p;

	list L;

	list_init(&L);

	for (i=0; i<10; ++i)
		list_push_back(&L, &a[i]);

	p = list_first(&L);
	while (p != list_tail(&L)) {
		printf("%d ", *p);
		p = list_next(&L, p);
	}
	printf("\n");

	list_sort(&L, cmp);

	p = list_first(&L);
	while (p != list_tail(&L)) {
		printf("%d ", *p);
		p = list_next(&L, p);
	}
	printf("\n");

	list_destroy(&L);

	return 0;
}

