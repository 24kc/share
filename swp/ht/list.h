#ifndef type
#error: no define "type", need [#define type ...] before [#include "list.h"]
#else

#include <stdio.h> /* FILE, fwrite, fread */
#include <stdlib.h> /* malloc, free */

/* 结点类型 */
typedef struct Node{
	type data; /* 结点中的数据 */
	struct Node *prev; /* 指向上一个结点 */
	struct Node *next; /* 指向下一个结点 */
}Node;

/* 双向链表 */
typedef struct list{
	Node *head; /* 头结点 */
	Node *tail; /* 尾结点 */
	int size; /* 元素数量 */
}list;

int list_init (list*); /* 初始化list, 失败返回0, 成功返回非0 */
void list_destroy (list*); /* 销毁list */
type* list_first (list*); /* 返回指向第一个元素的指针(头结点后一个) */
type* list_last (list*); /* 返回指向最后一个元素的指针(尾结点前一个) */
type* list_head (list*); /* 返回指向头结点的指针 */
type* list_tail (list*); /* 返回指向尾结点的指针 */
type* list_prev (list*, type *t); /* 返回指向t前一个元素的指针 */
type* list_next (list*, type *t); /* 返回指向t后一个元素的指针 */
int list_empty (list*); /* 如果链表为空(元素个数为0)则返回非0, 否则返回0 */
int list_size (list*); /* 返回链表元素个数 */
void list_clear (list*); /* 删除链表中所有元素 */
int list_insert (list*, type *t, const type *ct); /* t为指向链表中某元素的指针, 把ct插入到t前面 */
int list_erase (list*, type *t); /* t为指向链表中某元素的指针, 在链表中删除t指向的元素 */
int list_push_back (list*, const type *t); /* 把t插入到链表末尾 */
int list_push_front (list*, const type *t); /* 把t插入到链表开头 */
int list_pop_back (list*); /* 删除链表最后一个的元素 */
int list_pop_front (list*); /* 删除链表第一个的元素 */

void list_sort (list*, int (*cmp)(const type*, const type*));
	/* 排序: 根据cmp函数进行比较, 把链表中元素按从小到大排序 */
	/* cmp: 比较函数. */
		/* 若首个参数小于第二个,则返回负整数值; */
		/* 若首个参数大于第二个,则返回正整数值; */
		/* 若两参数相等，则返回零 */

/* 文件操作 */
int list_save (list*, const char *file_name);
	/* 创建file_name文件, 把链表数据写入, 若文件存在将清空原文件数据 */
	/* 打开文件失败返回0, 成功返回非0 */
	/* 可以用list_load读取此函数保存的数据 */
int list_load (list*, const char *file_name);
	/* 打开file_name文件, 读取list_save保存的链表数据 */
	/* 打开文件失败返回0, 成功返回非0 */

int list_write (list*, FILE *fp); /* 把链表数据写入fp */
int list_read (list*, FILE *fp); /* 从fp读取链表数据 */

int
list_init (list *L)
{
	L->head = (Node*) malloc ( sizeof(Node) );
	if ( ! L->head )
		return 0;
	L->tail = (Node*) malloc ( sizeof(Node) );
	if ( ! L->tail ) {
		free ( L->head );
		return 0;
	}
	L->head->prev = NULL;
	L->head->next = L->tail;
	L->tail->prev = L->head;
	L->tail->next = NULL;
	L->size = 0;
	return 1;
}

void
list_destroy (list *L)
{
	Node *node = L->head->next, *tmp_node = node;

	while (node != L->tail) {
		tmp_node = node->next;
		free(node);
		node = tmp_node;
	}

	free(L->head);
	free(L->tail);

	L->head = NULL;
	L->tail = NULL;
	L->size = 0;
}

type*
list_first (list *L)
{
	return &L->head->next->data;
}

type*
list_last (list *L)
{
	return &L->tail->prev->data;
}

type*
list_head (list *L)
{
	return &L->head->data;
}

type*
list_tail (list *L)
{
	return &L->tail->data;
}

type*
list_prev (list *L, type *t)
{
	Node *node = (Node*)t;
	if ( ! node->prev )
		return NULL;
	return &node->prev->data;
}

type*
list_next (list *L, type *t)
{
	Node *node = (Node*)t;
	if ( ! node->next )
		return NULL;
	return &node->next->data;
}

int
list_empty (list *L)
{
	if ( L->size )
		return 0;
	return 1;
}

int
list_size (list *L)
{
	return L->size;
}

void
list_clear (list *L)
{
	list_destroy(L);
	list_init(L);
}

int
list_insert (list *L, type *t, const type *ct)
{
	Node *node, *new_node;

	node = (Node*)t;
	if ( node == L->head )
		return 0;

	new_node = (Node*) malloc ( sizeof(Node) );
	if ( ! new_node )
		return 0;

	new_node->data = *ct;

	new_node->next = node;
	new_node->prev = node->prev;
	node->prev = new_node;
	new_node->prev->next = new_node;
	++L->size;

	return 1;
}

int
list_erase (list *L, type *t)
{
	Node *node = (Node*)t;
	if ( node == L->head || node == L->tail )
		return 0;

	node->next->prev = node->prev;
	node->prev->next = node->next;
	free(node);
	--L->size;

	return 1;
}


int
list_push_back (list *L, const type *t)
{
	return list_insert(L, list_tail(L), t);
}

int
list_push_front (list *L, const type *t)
{
	return list_insert(L, list_first(L), t);
}

int
list_pop_back (list *L)
{
	return list_erase(L, list_last(L));
}

int
list_pop_front (list *L)
{
	return list_erase(L, list_first(L));
}

int
list_write (list *L, FILE *fp)
{
	int r;
	type *t = list_first(L);

	r = fwrite(&L->size, sizeof(int), 1, fp);
	if ( r != 1 )
		return 0;
	while (t != list_tail(L)) {
		r = fwrite(t, sizeof(type), 1, fp);
		if ( r != 1 )
			return 0;
		t = list_next(L, t);
	}
	return 1;
}

int
list_read (list *L, FILE *fp)
{
	int i, r, size;
	type t;

	r = fread(&size, sizeof(int), 1, fp);
	if ( r != 1 )
		return 0;
	for (i=0; i<size; ++i) {
		r = fread(&t, sizeof(type), 1, fp);
		if ( r != 1 )
			return 0;
		list_push_back(L, &t);
	}
	return 1;
}

int
list_save (list *L, const char *file_name)
{
	int r;
	FILE *fp;
	fp = fopen(file_name, "w");
	if ( ! fp )
		return 0;
	r = list_write(L, fp);
	fclose(fp);
	return r;
}

int
list_load (list *L, const char *file_name)
{
	int r;
	FILE *fp;
	fp = fopen(file_name, "r");
	if ( ! fp )
		return 0;
	r = list_read(L, fp);
	if ( ! r )
		list_clear(L);
	fclose(fp);
	return r;
}

static int (*list_sort_origin_cmp) (const type*, const type*) = NULL;

static
int
list_sort_cmp (const void *p1, const void *p2)
{
	return list_sort_origin_cmp(*(const type**)p1, *(const type**)p2);
}

void
list_sort (list *L, int (*cmp)(const type*, const type*))
{
	int i, size;
	type *t, **array;
	Node *node;

	size = list_size(L);
	if ( size < 2 )
		return;

	array = (type**)malloc(sizeof(type*) * size);

	t = list_first(L);
	for (i=0; i<size; ++i) {
		array[i] = t;
		t = list_next(L, t);
	}

	list_sort_origin_cmp = cmp;
	qsort(array, size, sizeof(type*), list_sort_cmp);

	node = (Node*)array[0];
	L->head->next = node;
	node->prev = L->head;
	node->next = (Node*)array[1];

	node = (Node*)array[size-1];
	L->tail->prev = node;
	node->next = L->tail;
	node->prev = (Node*)array[size-2];

	for (i=1; i<size-1; ++i) {
		node = (Node*)array[i];
		node->prev = (Node*)array[i-1];
		node->next = (Node*)array[i+1];
	}

	free(array);
}

#undef type

#endif
