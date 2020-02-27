CC = gcc -Wall -pedantic

objs =
exes = 001
libs = -lm

001: 001.c $(objs) rsa.h
	$(CC) 001.c $(objs) -o 001 $(libs)

.PHONY: clean
clean:
	rm $(objs) $(exes)

