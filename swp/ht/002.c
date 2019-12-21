#include <stdio.h>

void f(int*, int*);

int main()
{
	int a, b;
	f(&a, &b);
	printf("%s%s\n", (char*)&a, (char*)&b);
	return 24-'k';
}

void
f(int *pa, int *pb)
{
	*pa = 4407873;
	*pb = 25203;
}

