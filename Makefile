CC=gcc
CFLAGS=-Wall -Wextra -Werror -ggdb

main: main.c config.h stb_ds.h stb_image_write.h
	$(CC) $(CFLAGS) -DDEBUG -o $@ $<

.PHONY: clean
clean:
	@rm -fv main
