#ifndef _STATIC_LIST_H_
#define _STATIC_LIST_H_

#ifndef type
#error: \
	No define "type", need [#define type ...] before [#include "static_list.h"]
#endif

typedef int sl_off_t; // Node offset type

typedef struct sl_node_t{
	type data;
	struct sl_node_t *prev;
	struct sl_node_t *next;
} sl_node_t; // Static linked list node

typedef struct {
	sl_node_t *baseptr; // Base address
	int size;
	sl_off_t boff; // Basic allocation offset
	sl_node_t *free; // a list store the released nodes
	int free_size;
} static_list;

#define OFF_NULL  (0)
#define _sl_node_init(p, l, r)  ( p->prev = l, p->next = r )
#define sl_node_init(p, l, r)  _sl_node_init((p), (l), (r))

void sl_prev_add(sl_node_t*, sl_node_t*);
void sl_next_add(sl_node_t*, sl_node_t*);
void sl_prev_del(sl_node_t*);
void sl_next_del(sl_node_t*);

static_list* sl_init (void*, int);
sl_node_t* sl_alloc (static_list*);
void sl_free (static_list*, sl_node_t*);
int sl_size (static_list*);
int sl_free_size (static_list*);

#endif
