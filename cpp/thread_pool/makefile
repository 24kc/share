CC = g++ -std=c++17 -Wall -pedantic
objs =
exes = 001
libs = -lpthread

001: 001.cpp $(objs) thread_pool.h
	$(CC) 001.cpp $(objs) -o 001 $(libs)

.PHONY: clean
clean:
	rm $(objs) $(exes)

