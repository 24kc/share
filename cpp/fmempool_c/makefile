CC = clang++ -Wall -pedantic
objs = fmempool.o
exes = 001

001: 001.cpp $(objs)
	$(CC) 001.cpp $(objs) -o 001

test: test.cpp $(objs)
	$(CC) test.cpp $(objs) -o test

fmempool.o: fmempool.h fmempool.cpp
	$(CC) fmempool.cpp -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

