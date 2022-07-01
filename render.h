#include <stdio.h>
#include <string.h>
#include "draw.h"

static bool isFirst = true;

// make box of size max length
void render(char *keyword, char *value, unsigned long tcol) {
	if (isFirst) {
		draw_tl_corner();
	} else {
		printf("│");
	}

	draw_hline(tcol - 2);

	if (isFirst) {
		draw_tr_corner();
		isFirst = false;
	} else {
		printf("│");
	}


	printf("\n│ \033[1m%s\033[0m ", keyword);

	int padding = tcol - strlen(keyword) - strlen(value) - 6;

	for (int i=0; i<padding; ++i) {
		if (15-i-strlen(keyword) == 0)
			printf("┃");
		else
			printf(" ");
	}

	printf(" %s ", value);
	printf("│\n");
}

void finish_render(unsigned long tcol) {
	draw_bl_corner();
	draw_hline(tcol - 2);
	draw_br_corner();
	printf("\n");
}