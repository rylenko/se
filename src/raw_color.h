#ifndef _RAW_COLOR_H
#define _RAW_COLOR_H

#include "buf.h"

/*
Colors for colored output. Contains only fg numbers. Bg applied when used.

See https://en.wikipedia.org/wiki/ANSI_escape_code#Colors.
*/
typedef enum {
	RAW_COLOR_BLACK = 30,
	RAW_COLOR_RED = 31,
	RAW_COLOR_GREEN = 32,
	RAW_COLOR_YELLOW = 33,
	RAW_COLOR_BLUE = 34,
	RAW_COLOR_MAGENTA = 35,
	RAW_COLOR_CYAN = 36,
	RAW_COLOR_WHITE = 37,
	RAW_COLOR_BRIGHT_BLACK = RAW_COLOR_BLACK + 60,
	RAW_COLOR_BRIGHT_RED = RAW_COLOR_RED + 60,
	RAW_COLOR_BRIGHT_GREEN = RAW_COLOR_GREEN + 60,
	RAW_COLOR_BRIGHT_YELLOW = RAW_COLOR_YELLOW + 60,
	RAW_COLOR_BRIGHT_BLUE = RAW_COLOR_BLUE + 60,
	RAW_COLOR_BRIGHT_MAGENTA = RAW_COLOR_MAGENTA + 60,
	RAW_COLOR_BRIGHT_CYAN = RAW_COLOR_CYAN + 60,
	RAW_COLOR_BRIGHT_WHITE = RAW_COLOR_WHITE + 60,
} RawColor;

/* Begins colored output with specified background and foreground. */
void raw_color_begin(Buf *, RawColor, RawColor);

/* Ends colored output. */
void raw_color_end(Buf *);

#endif /* _COLOR_H */
