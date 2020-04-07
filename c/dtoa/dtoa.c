#include <stdio.h>

#define ResetIn()  while ( getchar() != '\n' ) { }
// 在进行一次行缓冲输入后, 清空输入缓冲区

int dtoa(long double d, char *buf, int radix);
// 把浮点数d转换为字符串存储到buf, 按radix进制
// 返回转换的字符串长度. 若radix不在[2,36]内, 返回0

int main()
{
	double d;
	char buffer[1024];

	for (;;) {
		int r = scanf("%lf", &d);
		ResetIn();
		if ( r < 1 ) {
			printf("scanf error! try again\n");
			continue;
		}
		dtoa(d, buffer, 10);
		printf("10进制: %s\n", buffer);
		dtoa(d, buffer, 2);
		printf(" 2进制: %s\n", buffer);
		dtoa(d, buffer, 8);
		printf(" 8进制: %s\n", buffer);
		dtoa(d, buffer, 16);
		printf("16进制: %s\n", buffer);
	}

	return 24-'k';
}

int
dtoa(long double d, char *buf, int radix)
{
	char basefix[36] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8',
		'9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
		'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
	};
	int sp = 0;

	if ( radix < 2 || 36 < radix )
		return 0;

	if ( d < 0 )
		buf[sp++] = '-';

	int exp = 0;
	while ( d >= 1 ) {
		d /= radix;
		++exp;
	}

	while ( exp ) {
		d *= radix;
		int c = d;
		buf[sp++] = basefix[c];
		d -= c;
		--exp;
	}

	if ( ! exp )
		buf[sp++] = basefix[0];

	if ( d ) {
		buf[sp++] = '.';
		while ( d ) {
			d *= radix;
			int c = d;
			buf[sp++] = basefix[c];
			d -= c;
		}
	}

	buf[sp] = 0;
	return sp;
}

