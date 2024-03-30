#ifndef _WIN_H
#define _WIN_H

#include <stddef.h>
#include <sys/ioctl.h>
#include "file.h"
#include "line.h"

enum {
	STAT_ROWS_CNT = 1, /* Count of rows reserved for status */
};

/* Cursor's poisition in the window. */
typedef struct {
	unsigned short row;
	unsigned short col;
} WinCur;

/* Offset in the file for current view. */
typedef struct {
	size_t rows;
	size_t cols;
} WinOffset;

/* Information about what the user sees on the screen. */
typedef struct {
	File file; /* Opened file */
	WinOffset offset; /* Offset of current view */
	WinCur cur;
	struct winsize size; /* Window size */
} Win;

/* Clamps cursor to the line. Useful when moving between lines. */
void win_clamp_cur_to_line(Win *);

/* Closes the window. */
void win_close(Win *);

/*
Gets the count of characters by which the part of line is expanded using tabs.
The part of the line from the beginning to the passed column is considered.
*/
size_t win_exp_col(const Win *, size_t);

/* Used than expanded cursor goes off window. */
void win_fix_exp_cur_col(Win *);

/* Gets current line. */
Line *win_get_curr_line(const Win *);

/* Handles signal. */
void win_handle_signal(Win *, int);

/*
Opens window with file and initializes terminal. Do not forget to close it.
*/
void win_open(Win *, const char *, int, int);

#endif /* WIN_H */
