CC 			= gcc
CFLAGS		= -Wall -Wextra -pedantic -pedantic-errors
LDFLAGS    	=

all: clean test example

clean:
	rm -f *.bin

test:
	$(CC) $(CFLAGS) test.c JSON/*.c $(LDFLAGS) -o $@.bin

example:
	$(CC) $(CFLAGS) example.c JSON/*.c $(LDFLAGS) -o $@.bin

debug: clean test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./test.bin
