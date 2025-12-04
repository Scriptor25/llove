LLOVE = ../build/llove
LD = ld
GCC = gcc

.PHONY: all clean

all: example.launch fib.launch iterator.launch memory.launch operator.launch rays.launch test.launch

clean:
	- rm *.x

%.o: %.lov
	$(LLOVE) $^ -o $@

%.x: std/format.o std/typeinfo.o %.o
	$(GCC) $^ -o $@

%.launch: %.x
	- ./$<
