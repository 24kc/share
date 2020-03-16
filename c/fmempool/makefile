CC = clang++ -Wall -pedantic
objs = fmempool.o
exes = 001

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

fmempool.o: fmempool.h fmempool.c
	$(CC) fmempool.c -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

