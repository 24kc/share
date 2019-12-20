#include <stdio.h>

typedef struct{
	char data[32];
}string;

#define type string
#include "list.h"

int main() {
	int i;
	string s, *ps;
	list L, L2;

	list_init(&L);

	// 输入5个字符串, 不要太长
	for (i=0; i<5; ++i) {
		scanf("%31s", s.data);
		list_push_back(&L, &s);
	}

	// 遍历输出链表中存储的string
	printf("\n链表L:\n");
	ps = list_first(&L);
	while ( ps != list_tail(&L) ) {
		printf("%s\n", ps->data);
		ps = list_next(&L, ps);
	}

	// 把链表数据保存到 1.txt
	list_save(&L, "1.txt");

	// 初始化L2, 并从 1.txt 读取数据到L2
	list_init(&L2);
	list_load(&L2, "1.txt");

	// 遍历输出链表中存储的string
	printf("\n链表L2:\n");
	ps = list_first(&L2);
	while ( ps != list_tail(&L2) ) {
		printf("%s\n", ps->data);
		ps = list_next(&L2, ps);
	}

	list_destroy(&L);
	list_destroy(&L2);

	return 0;
}

