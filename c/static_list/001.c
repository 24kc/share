#include <stdio.h>
#include <stdlib.h>
#define type int
#include "static_list.h"

#define N  (1000)

int main()
{
	static_list *sl = sl_init(malloc(N), N);
	printf("sl_size = %d\n", sl_size(sl));

	int i;
	sl_node_t *list = sl_alloc(sl);
	list->data = 24;
	for (i=0; i<100; ++i) {
		sl_node_t *p = sl_alloc(sl);
		if ( ! p )
			break;
		p->data = i * 100;
		sl_next_add(list, p);
	}

	sl_node_t *p = list;
	do {
		printf("%d ", p->data);
		p = p->next;
	} while ( p );
	puts("");

	p = list->next;
	for (i=0; i<10; ++i) {
		sl_node_t *p1 = p->next;
		sl_prev_del(p1);
		sl_free(sl, p);
		p = p1;
	}

	p = list;
	do {
		printf("%d ", p->data);
		p = p->next;
	} while ( p );
	puts("");
}

