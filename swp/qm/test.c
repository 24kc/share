#include <stdio.h>

#define type int // 设置链表中元素类型为int
#include "list.h"

int main() {
	int i, n;
	int *p;
	list L; // 声明一个链表

	list_init(&L); // 初始化链表

	printf("输入5个数字: ");
	for (i=0; i<5; ++i) {
		scanf("%d", &n);
		list_push_back(&L, &n); // 把数字n插入到链表末尾
	}

	printf("输出链表中存储的数字:\n");
	p = list_first(&L); // 让p指向链表中第一个元素
	while ( p != list_tail(&L) ) { // 如果p不是尾结点
		printf("%d ", *p);
		p = list_next(&L, p); // 让p指向下一个元素
	}
	printf("\n");

	return 0;
}

