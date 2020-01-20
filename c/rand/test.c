#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double rand_double();

int main()
{
	srand(time(NULL));

	int i;
	for (i=0; i<10; ++i)
		printf("%g\n", rand_double());

	return 24-'k';
}

double
rand_double()
{
	int size = sizeof(double);
	char d[size];

	int i;
	for (i=0; i<size; ++i)
		d[i] = rand() & 0xff;

	return *(double*)d;
}

