CC = g++ -Wall -pedantic

objs = mempool.o my_new.o
exes = 001
libs = -lpthread

001: 001.cpp $(objs)
	$(CC) 001.cpp $(objs) -o 001 $(libs)

mempool.o: mempool.h mempool.cpp
	$(CC) mempool.cpp -c

my_new.o: my_new.h my_new.cpp
	$(CC) my_new.cpp -c

.PHONY: clean
clean:
	rm $(objs) $(exes)

