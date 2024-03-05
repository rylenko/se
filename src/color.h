#ifndef _COLOR_H
#define _COLOR_H

#include "buf.h"

/* Colors for colored output. */
typedef enum {
	/*
	https://en.wikipedia.org/wiki/ANSI_escape_code#Colors

	To get the background color we add 10.
	*/
	COLOR_BLACK = 30,
	COLOR_WHITE = 97,
} Color;

/* Begins colored output with specified background and foreground. */
void color_begin(Buf *, Color, Color);

/* Ends colored output. */
void color_end(Buf *);

#endif /* _COLOR_H */
