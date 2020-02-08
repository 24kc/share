#define type int
#include "static_list.h"

#include <stdlib.h>

static_list*
sl_init(void *mem, int size)
{
	if ( size < sizeof(static_list) + sizeof(sl_node_t) )
		return NULL;

	static_list *sl = (static_list*)mem;

	sl->baseptr = (sl_node_t*)(mem + sizeof(static_list));
	sl->size = ((mem + size) - (void*)sl->baseptr) / sizeof(sl_node_t);
	sl->baseptr -= 1; // for OFF_NULL

	sl->boff = OFF_NULL;
	sl->free = sl_alloc(sl);
	sl->free_size = 0;

	sl_node_init(sl->free, NULL, NULL);

	return sl;
}

sl_node_t*
sl_alloc(static_list *sl)
{
	if ( sl->boff < sl->size )
		return sl->baseptr + (++sl->boff);
	if ( sl->free->next ) {
		sl_node_t *p = sl->free->next;
		sl_next_del(sl->free);
		--sl->free_size;
		return p;
	}
	return NULL;
}

void
sl_free(static_list *sl, sl_node_t *p)
{
	sl_next_add(sl->free, p);
	++sl->free_size;
}

int
sl_size(static_list *sl)
{
	return sl->size - 1;
}

int
sl_free_size(static_list *sl)
{
	return sl->free_size;
}


void
sl_prev_add(sl_node_t *p, sl_node_t *p1)
{
	p1->prev = p->prev;
	p1->next = p;
	p->prev = p1;
	if ( p1->prev )
		p1->prev->next = p1;
}

void
sl_next_add(sl_node_t *p, sl_node_t *p1)
{
	p1->next = p->next;
	p1->prev = p;
	p->next = p1;
	if ( p1->next )
		p1->next->prev = p1;
}

void
sl_prev_del(sl_node_t *p)
{
	p->prev = p->prev->prev;
	if ( p->prev )
		p->prev->next = p;
}

void
sl_next_del(sl_node_t *p)
{
	p->next = p->next->next;
	if ( p->next )
		p->next->prev = p;
}


