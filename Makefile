CFLAGS = -Wall -Werror -ansi

.PHONY: all
all: mkdir huff

.PHONY: mkdir
mkdir:
	mkdir -p build bin

.PHONY: huff
huff: bin/huff_c bin/huff_e

bin/huff_c: build/main_c.o build/huff.o build/errhand.o build/bitio.o
	cc -o $@ $^

bin/huff_e: build/main_e.o build/huff.o build/errhand.o build/bitio.o
	cc -o $@ $^

build/main_c.o: src/main_c.c
	cc $(CLFAGS) -c $^ -o $@

build/main_e.o: src/main_e.c
	cc $(CLFAGS) -c $^ -o $@

build/huff.o: src/huff.c
	cc $(CLFAGS) -c $^ -o $@

build/errhand.o: src/errhand.c
	cc $(CLFAGS) -c $^ -o $@

build/bitio.o: src/bitio.c
	cc $(CLFAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf build bin
