CC=gcc
CFLAGS=-Wall -Wextra -Werror -ggdb

main: main.c
	$(CC) $(CFLAGS) -DDEBUG -o $@ $^

.PHONY: clean
clean:
	@rm -fv main
