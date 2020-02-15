ifdef s
CC = gcc -Wall -pedantic-errors
else
CC = gcc -Wall
endif

objs = mempool.o

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

002: 002.c $(objs)
	$(CC) 002.c $(objs) -o 002

mempool.o: mempool.h mempool.c
	$(CC) mempool.c -c

.PHONY: clean
clean:
	rm $(objs) 001 002

