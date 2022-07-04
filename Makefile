PROJECT := about
CC	:= gcc
SDIR	:= src
BDIR	:= /usr/local/bin
TARGET	:= about
CFLAGS = -g -Wall -lmagic

about:	$(addprefix $(SDIR)/, main.c)
	mkdir -p build
	$(CC) $< $(CFLAGS) -o $(addprefix $(BDIR)/, $@)

.PHONY: clean

clean:
	rm $(TARGETS)

