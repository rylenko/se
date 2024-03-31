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

/*
Working with a window is divided into two cases: working with internal and
rendered (or expanded) content.

For example, when we consider a tab as one character, then most likely we are
working with internal content, and when we consider a tab as several
characters that the user can see, then we are working with rendered content.

In structure, the offset and cursor are for internal content. The file contains
a method for getting the index of a expanded column from an internal column, a
method for clamping the internal column in the rendered window, and others.
*/
typedef struct {
	File file; /* Opened file */
	WinOffset offset; /* Offset of current view. Counts tabs as 1 character */
	WinCur cur; /* Pointer to window's content. Counts tabs as 1 character */
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
size_t win_exp_col(const Line *, size_t);

/*
Fixes expanded cursor column for current line. Used than expanded cursor goes
off window, but the internal cursor is still here.
*/
void win_fix_exp_cur_col(Win *);

/* Gets current line. */
Line *win_get_curr_line(const Win *);

/* Gets line by its index. */
Line *win_get_line(const Win *, size_t);

/* Handles signal. */
void win_handle_signal(Win *, int);

/*
Opens window with file and initializes terminal. Do not forget to close it.
*/
void win_open(Win *, const char *, int, int);

#endif /* WIN_H */
