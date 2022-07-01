#include<stdio.h>

void draw_hline(int len) {
	int i;

	for (i=0; i<len; ++i) {
		printf("%s", "─");
	}
}

void draw_tl_corner(void) {
	printf("%s", "┌");
}

void draw_tr_corner(void) {
	printf("%s", "┐");
}

void draw_bl_corner(void) {
	printf("%s", "└");
}

void draw_br_corner(void) {
	printf("%s", "┘");
}
