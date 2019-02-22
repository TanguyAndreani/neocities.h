CC=gcc # clang
FLAGS=-lcurl -ljson-c -I/usr/include/json-c
DEBUG=-g -DNEOCITIES_DEBUG

all: example

example: example.c neocities.h dtparser.c dtparser.h
	$(CC) -o $(<:.c=) dtparser.c $< $(FLAGS) $(DEBUG)

clean:
	rm -rf ./example