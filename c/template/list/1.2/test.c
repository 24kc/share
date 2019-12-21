#include <stdio.h>

/* 设置元素类型为int */
#define type int
#include "list.h"

int cmp(const int *p1, const int *p2)
{
	return (*p1 > *p2) - (*p1 < *p2);
/*
	比较函数, 从小到大排序
	改成 return (*p1 < *p2) - (*p1 > *p2); 则从大到小排序
*/
}

int main() {
	int a[10] = {85, 60, 90, 89, 9, 49, 92, 76, 95, 55};
	int i, *p;

	list L; /* 链表结构体 */

	list_init(&L); /* 初始化链表 */

	/* 把数据插入到链表中 */
	for (i=0; i<10; ++i)
		list_push_back(&L, &a[i]);

	/* 遍历输出链表 */
	p = list_first(&L);
	while (p != list_tail(&L)) {
		printf("%d ", *p);
		p = list_next(&L, p);
	}
	printf("\n");

	/* 根据cmp比较函数给链表排序 */
	list_sort(&L, cmp);

	/* 遍历输出链表 */
	p = list_first(&L);
	while (p != list_tail(&L)) {
		printf("%d ", *p);
		p = list_next(&L, p);
	}
	printf("\n");

	list_destroy(&L);

	return 0;
}

