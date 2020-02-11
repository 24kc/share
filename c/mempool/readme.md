# mempool  

### `mempool* mp_init (void*, int);`  
### `void mp_destroy (mempool*);`  

## `void* mp_alloc (mempool*, int);`  
## `void* mp_realloc (mempool*, void*, int);`  
## `void mp_free (mempool*, void*);`  

```
mempool *mp = mp_init(malloc(1000), 1000);

void *p = mp_alloc(mp, 300);
if ( ! p ) {
	puts("Out of memory");
	return;
}

p = mp_realloc(mp, p, 480);
if ( ! p ) {
	puts("Out of memory");
	return;
}

mp_free(mp, p);

mp_destroy(mp); // <=> free(mp);
```

