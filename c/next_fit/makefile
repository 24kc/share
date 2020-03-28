CC = gcc -Wall -pedantic
objs = next_fit.o
exes = 001

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

next_fit.o: next_fit.h next_fit.c
	$(CC) next_fit.c -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

