#include <stdio.h>

void change(int[2]);

int main()
{
	int a[2] = {24, 'k'};
	change(a);
	printf("%d %d\n", a[0], a[1]);
	return 0;
}


void change(int a[2])
{
	int tmp = a[0];
	a[0] = a[1];
	a[1] = tmp;
}

