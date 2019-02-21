CC=gcc # clang
FLAGS=-lcurl -ljson-c -I/usr/include/json-c
DEBUG=-DNEOCITIES_DEBUG

all: example

example: example.c
	$(CC) -o $(<:.c=) $< $(FLAGS) $(DEBUG)

clean:
	rm -f ./example