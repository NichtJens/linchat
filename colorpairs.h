#ifndef LINCHAT_COLORPAIRS_H
#define LINCHAT_COLORPAIRS_H

#include <curses.h>

/*
From ncurses.h:
#define COLOR_BLACK     0
#define COLOR_RED       1
#define COLOR_GREEN     2
#define COLOR_YELLOW    3
#define COLOR_BLUE      4
#define COLOR_MAGENTA   5
#define COLOR_CYAN      6
#define COLOR_WHITE     7
*/

#define MIN_COLOR COLOR_BLACK
#define MAX_COLOR COLOR_WHITE
#define NUMBER_OF_COLORS (MAX_COLOR - MIN_COLOR + 1)  // should be equal to COLORS from ncurses.h
#define COLOR_TYPE NCURSES_COLOR_T  // defined as short in ncurses.h

// Can one get this from the terminal?
#define STD_BG_COLOR COLOR_BLACK
#define STD_FG_COLOR COLOR_WHITE

// Not really good here, but anyway
#define USRLST_BG_COLOR COLOR_BLUE
#define USRLST_FG_COLOR COLOR_WHITE


void init_color_pairs();
int pair_number(COLOR_TYPE fg_color, COLOR_TYPE bg_color);
COLOR_TYPE number_to_color(int number);

#endif  // LINCHAT_COLORPAIRS_H
