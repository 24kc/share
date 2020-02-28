CC = g++ -Wall -pedantic

objs = mempool.o
exes = 001

001: 001.cpp $(objs)
	$(CC) 001.cpp $(objs) -o 001

mempool.o: mempool.h mempool.cpp
	$(CC) mempool.cpp -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

