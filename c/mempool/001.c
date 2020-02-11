#include <stdio.h>
#include <stdlib.h>
#define type int
#include "mempool.h"
#include <string.h>

#define N  (1200)

void print_mp(mempool*); // 输出内存池信息

int main()
{
	mempool *mp = mp_init(malloc(N), N);
	mp_check(mp);
	print_mp(mp);

	void *p, *p1;

	p = mp_alloc(mp, 400);
	p1 = mp_alloc(mp, 416);
	mp_check(mp);
	print_mp(mp);

	p = mp_realloc(mp, p, 14);
	mp_check(mp);
	print_mp(mp);

	int n;
	mp_free(mp, p1);
	p = mp_max_block(mp, &n, 1);
	mp_check(mp);
	print_mp(mp);
	printf("%p %d\n", p, n);

	mempool *mp1 = mp_init(p, n);
	mp_check(mp1);
	print_mp(mp1);
}

int list_prev_num(mp_node_t*);
int list_next_num(mp_node_t*);

void
print_mp(mempool *mp)
{
	int flag = 0;
	printf("[capacity] = %d\n", mp_capacity(mp));
	for (int i=mp->list_num-1; i>=0; --i) {
		int nalloc = list_prev_num(&mp->list[i]);
		int nfree = list_next_num(&mp->list[i]);
		if ( ! nalloc && ! nfree )
			continue;
		if ( flag )
			printf("  ");
		flag = 1;
		printf("{<%d>", mp->list[i].capacity);
		if ( nfree ) {
			printf("f(%d)", nfree);
		}
		if ( nalloc ) {
			printf("a(%d)", nalloc);
			mp_node_t *ml = &mp->list[i];
			printf("[");
			while ( ml->prev ) {
				ml = ml->prev;
				printf("%d", ml->size);
				if ( ml->prev )
					printf(",");
			}
			printf("]");
		}
		printf("}");
	}
	puts("");
}

int
list_prev_num(mp_node_t *ml)
{
	int n = 0;
	while ( ml->prev ) {
		++n;
		ml = ml->prev;
	}
	return n;
}

int
list_next_num(mp_node_t *ml)
{
	int n = 0;
	while ( ml->next ) {
		++n;
		ml = ml->next;
	}
	return n;
}

