#include <stdio.h>
#include "getch.h"

int main()
{
	char s[1024];
	init_getch();
	set_getche();
	fgets(s, 5, stdin);
	printf("%s\n", s);
	end_getch();
	return 24-'k';
}

