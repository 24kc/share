#include <stdio.h>
#include <stdlib.h>
#include "mempool.h"

int main()
{
	char a[2300];
	mempool *mp = mp_init(a, sizeof(a));
	mp->nothrow = 0; // 设置该内存池 alloc/realloc 内存不足时抛出异常

	int i,n;
	char * buffer;

	printf ("How long do you want the string? ");
	scanf ("%d", &i);

	buffer = (char*) mp_alloc (mp, i+1);
	mp_check(mp); // 检查内存池是否正确, 仅供调试

	for (n=0; n<i; n++)
		buffer[n]=rand()%26+'a';
	buffer[i]='\0';

	printf ("Random string: %s\n",buffer);
	mp_free (mp, buffer);
	mp_check(mp); // 检查内存池是否正确, 仅供调试

	return 0;
}	

