CC = g++ -std=c++17 -Wall -pedantic
objs = logger.o
exes = 001
libs = -lpthread

logger.o: logger.h logger.cpp
	$(CC) logger.cpp -c $(libs)

001: 001.cpp $(objs)
	$(CC) 001.cpp $(objs) -o 001 $(libs)

.PHONY: clean
clean:
	rm $(objs) $(exes)

