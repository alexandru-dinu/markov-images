CC = gcc
CFLAGS = -Wall -Wextra -Werror -ggdb
HEADERS = cifar.h config.h markov.h stb_ds.h stb_image_write.h utils.h

.PHONY: run clean

main: main.c $(HEADERS)
	$(CC) $(CFLAGS) -DDEBUG -o $@ $<

run: main
	./main
	feh -g 512x512 -Z ./data/output

clean:
	@rm -fv main
