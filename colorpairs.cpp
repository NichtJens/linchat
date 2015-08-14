#include "colorpairs.h"
#include <curses.h>

// Init all color pairs from the basic set of colors
void init_color_pairs()
{
	for (COLOR_TYPE i=MIN_COLOR; i<=MAX_COLOR; ++i) {
	for (COLOR_TYPE j=MIN_COLOR; j<=MAX_COLOR; ++j) {
		init_pair(pair_number(i, j), i, j);
	}}
}

// Calculate a pair number from two color numbers
int pair_number(COLOR_TYPE fg_color, COLOR_TYPE bg_color)
{
	if (fg_color < MIN_COLOR) fg_color = MIN_COLOR;
	if (fg_color > MAX_COLOR) fg_color = MAX_COLOR;
	if (bg_color < MIN_COLOR) bg_color = MIN_COLOR;
	if (bg_color > MAX_COLOR) bg_color = MAX_COLOR;
	return bg_color * NUMBER_OF_COLORS + fg_color;
}

// Convert any number to a color number, by modulo maximum number
COLOR_TYPE number_to_color(int number)
{
	return (number + 1) % (NUMBER_OF_COLORS - 2) + MIN_COLOR;
}

