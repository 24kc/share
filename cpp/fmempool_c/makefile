CC = g++ -Wall -pedantic
objs = fmempool.o
exes = 001

001: 001.cpp $(objs)
	$(CC) 001.cpp $(objs) -o 001

fmempool.o: fmempool.h fmempool.cpp
	$(CC) fmempool.cpp -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

