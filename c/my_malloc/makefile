CC = gcc -Wall -pedantic
objs = mempool.o my_malloc.o
exes = 001

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

mempool.o: mempool.h mempool.c
	$(CC) mempool.c -c

my_malloc.o: my_malloc.h my_malloc.c
	$(CC) my_malloc.c -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

