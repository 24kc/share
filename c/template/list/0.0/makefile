.PHONY: test
test: test.c list.h
	gcc test.c
	@echo make OK

.PHONY: test-Wall
test-Wall: test.c list.h
	gcc test.c -Wall -pedantic-errors -std=c89
	@echo make OK

.PHONY: clean
clean: a.out
	rm a.out

