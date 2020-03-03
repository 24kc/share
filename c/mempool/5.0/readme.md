# mempool  

### `mempool* mp_init (void *mem, int size, int flag);`  

## `void* mp_alloc (mempool*, int size);`  
## `void* mp_realloc (mempool*, void *mem, int size);`  
## `void mp_free (mempool*, void *mem);`  

```
mempool *mp = mp_init(malloc(1000), 1000, MP_THROW);

void *p = mp_alloc(mp, 300);
p = mp_realloc(mp, p, 480);

mp_free(mp, p);

free(mp);
```

