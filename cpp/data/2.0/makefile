CC := g++ -Wall -pedantic-errors

.PHONY: all
all: reset select delete

.PHONY: reset
reset: reset.cpp
	$(CC) reset.cpp -o reset

.PHONY: select
select: select.cpp
	$(CC) select.cpp -o select

.PHONY: delete
delete: delete.cpp
	$(CC) delete.cpp -o delete

.PHONY: clean
clean:
	rm reset select delete data.akmdb

