CC = gcc -Wall -pedantic
objs = first_fit.o
exes = 001

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

first_fit.o: first_fit.h first_fit.c
	$(CC) first_fit.c -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

