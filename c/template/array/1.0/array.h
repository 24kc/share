#ifndef type
#error: no define "type", need [#define type ...] before [#include "array.h"]
#else

#include <stdio.h> // FILE, fwrite, fread
#include <stdlib.h> // malloc, free
#include <string.h> // memset
#include <stdarg.h> // va_list

typedef struct {
	type *data;
	int length;
	int capacity;
}array;

array new_array(int n, ...); // 初始化数组, 并设置n个元素
int array_add (array *arr, const type t); // 增加一个元素到数组末尾
// a.length 数组长度
// a.data[i] 访问或设置元素
int array_insert (array *arr, int index, const type t); // 在index处插入一个元素t
int array_insert_many (array *arr, int index, int n, ...); // 插入多个元素
int array_remove (array*, int index); // 删除index处元素
int array_set_length (array*, int new_length); // 设置数组长度
void array_clear (array*); // 删除数组内所有元素
void delete_array(array*); // 释放数组内存

static int array_reserve (array*, int new_cap); // 扩容数组
static int array_vinsert_many (array*, int index, int n, va_list ap); // 通过va_list插入多个元素

array
new_array (int n, ...)
{
	array a;

	va_list ap;
	va_start(ap, n);

	a.data = (type*) malloc (10 * sizeof(type));
	a.length = 0;
	a.capacity = 10;

	array_vinsert_many(&a, 0, n, ap);

	va_end(ap);

	return a;
}

void
delete_array (array *arr)
{
	free(arr->data);
	arr->data = NULL;
	arr->length = 0;
	arr->capacity = 0;
}

int
array_reserve (array *arr, int new_cap)
{
	type *p;
	if ( new_cap > arr->capacity ) {
		p = (type*) realloc (arr->data, new_cap * sizeof(type));
		if ( ! p )
			return 0;
		arr->data = p;
		arr->capacity = new_cap;
	}
	return 1;
}

int
array_set_length (array *arr, int new_length)
{
	if ( new_length < 0 )
		return 0;
	if ( new_length > arr->length ) {
		if ( array_reserve(arr, new_length) )
			memset(arr->data + arr->length, 0, (new_length - arr->length) * sizeof(type));
		else
			return 0;
	}
	arr->length = new_length;
	return 1;
}

int
array_insert (array *arr, int index, const type t)
{
	type *p;
	if ( index < 0 || index > arr->length )
		return 0;
	if ( arr->length == arr->capacity && ! array_reserve(arr, arr->capacity << 1) )
		return 0;
	p = arr->data + index;
	memmove(p+1, p, (arr->length - index) * sizeof(type));
	*p = t;
	++arr->length;
	return 1;
}

int
array_insert_many (array *arr, int index, int n, ...)
{
	int r;
	va_list ap;
	va_start(ap, n);
	r = array_vinsert_many(arr, index, n, ap);
	va_end(ap);
	return r;
}

int
array_vinsert_many (array *arr, int index, int n, va_list ap)
{
	int i;
	int new_length = arr->length + n;
	type *p;

	if ( index < 0 || index > arr->length || n < 0 )
		return 0;
	if ( new_length > arr->capacity && ! array_reserve(arr, new_length) )
		return 0;
	p = arr->data + index;
	memmove(p+n, p, (arr->length - index) * sizeof(type));
	for (i=0; i<n; ++i) {
// char error
		p[i] = va_arg(ap, type);
	}
	arr->length += n;
	return 1;
}

int
array_remove (array *arr, int index)
{
	type *p;
	if ( index < 0 || index >= arr->length )
		return 0;
	p = arr->data + index;
	--arr->length;
	memmove(p, p+1, (arr->length - index) * sizeof(type));
	return 1;
}

int
array_add (array *arr, const type t)
{
	return array_insert(arr, arr->length, t);
}

void
array_clear (array *arr)
{
	array_set_length(arr, 0);
}

#undef type

#endif

