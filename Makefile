CC = gcc
CFLAGS = -Wall -Wextra -Werror -ggdb
HEADERS = cifar.h config.h markov.h mnist.h stb_ds.h stb_image_write.h utils.h
SRC = main.c $(HEADERS)

.PHONY: run clean

main: $(SRC)
	$(CC) $(CFLAGS) -DDEBUG -o $@ $<

run: main
	./main
	feh -g 512x512 -Z ./data/output

tags: $(SRC)
	ctags -R --exclude=.git --exclude=data

clean:
	@rm -fv main
