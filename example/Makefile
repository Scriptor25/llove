LLOVE = ../build/llove
LD = ld
GCC = gcc

STDLIB = std/format.o std/typeinfo.o

.PHONY: clean

clean:
	rm *.x

%.o: %.lov
	$(LLOVE) $^ -o $@

%.x: %.o $(STDLIB)
	$(GCC) $^ -o $@
