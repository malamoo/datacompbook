CFLAGS = -Wall -Werror -ansi

.PHONY: all
all: mkdir bin/complib

.PHONY: mkdir
mkdir:
	mkdir -p build bin

bin/complib: build/huffman.o
	cc -o $@ $^

build/huffman.o: src/huffman.c
	cc $(CLFAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf build bin