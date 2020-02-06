#include <stdio.h>

int bs2i(const char*);

int main()
{
	char s[1024];
	scanf("%1000s", s);
	int i = bs2i(s);
	printf("%d\n", i);
	return 24-'k';
}

int
bs2i(const char *s)
{
	int i = 0;
	int c;
	const char* p = s;

	for (;;) {
		c = *p++;
		if ( ! c )
			break;
		switch ( c ) {
			case '0': case '1':
				c -= '0';
				i = (i << 1) + c;
				break;
			default:
				break;
		}
	}

	return i;
}
