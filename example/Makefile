LOV = ../build/llove
CC = clang

.PHONY: all clean

all: callable.launch example.launch fib.launch iterator.launch memory.launch operator.launch rays.launch test.launch

clean:
	- rm *.x

%.o: %.lov
	$(LOV) $^ -o $@

%.x: std/format.o std/typeinfo.o %.o
	$(CC) $^ -o $@

%.launch: %.x
	- ./$<
