#include <stdio.h>
#include <stdlib.h>
#define type int
#include "mempool.h"

#define N  (1200)

void print_mp(mempool*); // 输出内存池信息

int main()
{
	mempool *mp = mp_init(malloc(N), N);
	mp_check(mp);
	print_mp(mp);

	puts("\nint *p = (int*)mp_malloc(mp, 8);\n");
	int *p = (int*)mp_malloc(mp, 8);
	mp_check(mp);

	print_mp(mp);

	puts("\nmp_free(mp, p);\n");
	mp_free(mp, p);
	mp_check(mp);

	print_mp(mp);
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
			printf(", ");
		flag = 1;
		printf("{<%d> ", mp->list[i].capacity);
		if ( nfree ) {
			printf("f(%d)", nfree);
			if ( nalloc )
				putchar(' ');
		}
		if ( nalloc ) {
			printf("a(%d)", nalloc);
			mp_node_t *ml = &mp->list[i];
			printf("[");
			while ( ml->prev ) {
				ml = ml->prev;
				printf("%d", ml->size);
				if ( ml->prev )
					printf(", ");
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

