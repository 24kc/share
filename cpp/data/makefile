CC := g++ -Wall -pedantic-errors

basic_mempool: basic_mempool.o
	

basic_mempool.o: basic_mempool.h basic_mempool.cpp
	$(CC) basic_mempool.cpp -c

.PHONY: clean
clean:
	rm *.o

