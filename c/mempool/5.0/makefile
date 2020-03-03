CC = gcc -Wall -pedantic

exes = 001
objs = mempool.o

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

mempool.o: mempool.h mempool.c
	$(CC) mempool.c -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

