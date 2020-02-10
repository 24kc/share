CC = gcc -Wall
objs = static_list.o

.PHONY: it
it: 001

001: 001.c $(objs)
	$(CC) 001.c $(objs) -o 001

static_list.o: static_list.h static_list.c
	$(CC) static_list.c -c

.PHONY: clean
clean:
	rm $(objs)

