PROJECT := about
CC	:= gcc

about:
	$(CC) src/main.c -lmagic -o about
	mv about /usr/local/bin

