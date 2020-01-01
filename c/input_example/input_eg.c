#include <stdio.h>

#define ResetIn()	while (getchar() != '\n') {}
// 清空输入缓冲区

int
main()
{
	int i, r;

	for (;;) {
		printf("请输入一个整数: ");
		r = scanf("%d", &i);
		ResetIn();
		if (r != 1) {
			printf("\n输入格式错误!\n\n");
			continue;
		}
		printf("\nscanf OK: i = %d\n\n", i);
	}

	return 0;
}
