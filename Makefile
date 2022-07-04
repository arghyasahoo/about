PROJECT := about
CC	:= gcc

about:
	$(CC) main.c -lmagic -o about
	mv about /usr/local/bin

