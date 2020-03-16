CC = gcc -Wall -pedantic
objs = bitmap.o
exes = 001

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

bitmap.o: bitmap.h bitmap.c
	$(CC) bitmap.c -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

