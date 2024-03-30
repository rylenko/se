#ifndef _WIN_H
#define _WIN_H

#include <stddef.h>
#include <sys/ioctl.h>
#include "file.h"
#include "line.h"

enum {
	WIN_STAT_ROWS_CNT = 1, /* Count of rows reserved for status */
};

/* Information about what the user sees on the screen. */
typedef struct {
	File file; /* Opened file */
	size_t top_line_idx; /* Index of the top line in the window */
	size_t curr_line_idx; /* Index of the current line in the window */
	size_t curr_line_cont_idx; /* Content's index of current line */
	size_t curr_line_render_idx; /* Render's index of current line */
	size_t bot_line_idx; /* Index of the bottom line. Set after redraw */
	struct winsize size; /* Window size */
} Win;

/* Closes the window. */
void win_close(Win *);

/* Handles signal. */
void win_handle_signal(Win *, int);

/*
Opens window with file and initializes terminal. Do not forget to close it.
*/
void win_open(Win *, const char *, int, int);

#endif /* WIN_H */
