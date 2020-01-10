#include <stdio.h>

typedef struct{
	int A, B, C, s, b;
	char *p;
}STU;

int main()
{
	STU *stu = NULL;
	printf("%p\n", (void*)&stu->p);
}

