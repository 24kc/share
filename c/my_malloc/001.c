#include <stdio.h>
#include "my_malloc.h"

int main()
{
	char a[1200];
	_mp = mp_init(a, sizeof(a), MP_THROW);

	int i, n;
	char *buffer;

	printf("How long do you want the string? ");
	scanf("%d", &i);

	buffer = (char*)malloc(i+1);
	mp_print(_mp);

	for (n=0; n<i; n++)
		buffer[n] = rand() % 26 + 'a';
	buffer[i] = '\0';

	printf("Random string: %s\n", buffer);
	free(buffer);

	mp_print(_mp);

	return 0;
}	

