#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#ifndef MP_MIN_BLOCK
#define MP_MIN_BLOCK  (32)
// 2^n >= 32
#endif

typedef struct mp_node_t{
	int size; // Application size
	int capacity; // Actual capacity
	struct mp_node_t *prev;
	struct mp_node_t *next;
} mp_node_t; // mempool node

typedef struct {
	mp_node_t *list; // some lists store the released nodes
	int list_num;
	void *begin, *end; // mempool
} mempool;

mempool* mp_init (void*, int);
void mp_destroy (mempool*);

void* mp_malloc (mempool*, int);
void* mp_realloc (mempool*, void*, int);
void mp_free (mempool*, void*);

int mp_capacity (mempool*);
void* mp_max_block (mempool*, int*, int);

void mp_check(mempool*);

#endif
