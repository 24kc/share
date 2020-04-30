CC = g++ -std=c++17 -Wall -pedantic
objs =
exes = 001
libs =

.PHONY: all
all: 001

.PHONY: run
run:
	@./001

001: 001.cpp $(objs) Mean.h
	$(CC) 001.cpp $(objs) -o 001 $(libs)

.PHONY: clean
clean:
	rm $(objs) $(exes)
