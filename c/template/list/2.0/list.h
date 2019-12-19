#ifndef type
#error: no define "type", need [#define type ...] before [#include "list.h"]
#else

#include <stdio.h> // FILE, fwrite, fread
#include <stdlib.h> // malloc, free

// 结点类型
typedef struct Node{
	type data; // 结点中的数据
	struct Node *prev; // 指向上一个结点
	struct Node *next; // 指向下一个结点
}Node;

// 双向链表
typedef struct list{
	Node *head; // 头结点
	Node *tail; // 尾结点
	int size; // 元素数量
}list;

int list_init (list*); // 初始化list, 失败返回0, 成功返回非0
void list_destroy (list*); // 销毁list

int
list_init (list *this)
{
	this->head = (Node*) malloc ( sizeof(Node) );
	if ( ! this->head )
		return 0;
	this->tail = (Node*) malloc ( sizeof(Node) );
	if ( ! this->tail ) {
		free ( this->head );
		return 0;
	}
	this->head->prev = NULL;
	this->head->next = this->tail;
	this->tail->prev = this->head;
	this->tail->next = NULL;
	this->size = 0;
	return 1;
}

void
list_destroy (list *this)
{
	Node *node = this->head->next, *tmp_node = node;

	while (node != this->tail) {
		tmp_node = node->next;
		free(node);
		node = tmp_node;
	}

	free(this->head);
	free(this->tail);

	this->head = NULL;
	this->tail = NULL;
	this->size = 0;
}

type*
list_first (list *this)
{
	return &this->head->next->data;
}

type*
list_last (list *this)
{
	return &this->tail->prev->data;
}

type*
list_head (list *this)
{
	return &this->head->data;
}

type*
list_tail (list *this)
{
	return &this->tail->data;
}

type*
list_prev (list *this, type *t)
{
	Node *node = (Node*)t;
	if ( ! node->prev )
		return NULL;
	return &node->prev->data;
}

type*
list_next (list *this, type *t)
{
	Node *node = (Node*)t;
	if ( ! node->next )
		return NULL;
	return &node->next->data;
}

int
list_empty (list *this)
{
	if ( this->size )
		return 0;
	return 1;
}

int
list_size (list *this)
{
	return this->size;
}

void
list_clear (list *this)
{
	list_destroy(this);
	list_init(this);
}

int
list_insert (list *this, type *t, const type *ct)
{
	Node *node = (Node*)t;
	if ( node == this->head )
		return 0;

	Node *new_node = (Node*) malloc ( sizeof(Node) );
	if ( ! new_node )
		return 0;

	new_node->data = *t;

	new_node->next = node;
	new_node->prev = node->prev;
	node->prev = new_node;
	new_node->prev->next = new_node;
	++this->size;

	return 1;
}

int
list_erase (list *this, type *t)
{
	Node *node = (Node*)t;
	if ( node == this->head || node == this->tail )
		return 0;

	node->next->prev = node->prev;
	node->prev->next = node->next;
	free(node);
	--this->size;

	return 1;
}


int
list_push_back (list *this, const type *t)
{
	return list_insert(this, list_tail(this), t);
}

int
list_push_front (list *this, const type *t)
{
	return list_insert(this, list_first(this), t);
}

int
list_pop_back (list *this)
{
	return list_erase(this, list_last(this));
}

int
list_pop_front (list *this)
{
	return list_erase(this, list_first(this));
}

void
list_write (list *this, FILE *fp)
{
	type *t = list_first(this);

	fwrite(&this->size, sizeof(int), 1, fp);
	while (t != list_tail(this)) {
		fwrite(t, sizeof(type), 1, fp);
		t = list_next(this, t);
	}
}

void
list_read (list *this, FILE *fp)
{
	int i, size;
	type t;

	fread(&size, sizeof(int), 1, fp);
	for (i=0; i<size; ++i) {
		fread(&t, sizeof(type), 1, fp);
		list_push_back(this, &t);
	}
}

int
list_save (list *this, const char *file_name)
{
	FILE *fp;
	fp = fopen(file_name, "w");
	if ( ! fp )
		return 0;
	list_write(this, fp);
	fclose(fp);
	return 1;
}

int
list_load (list *this, const char *file_name)
{
	FILE *fp;
	fp = fopen(file_name, "r");
	if ( ! fp )
		return 0;
	list_read(this, fp);
	fclose(fp);
	return 1;
}

#undef type

#endif
