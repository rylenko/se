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

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "cfg.h"
#include "esc.h"
#include "file.h"
#include "line.h"
#include "lines.h"
#include "math.h"
#include "term.h"
#include "win.h"
#include "word.h"

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

/* Window parameters. */
struct Win {
	File *file; /* Opened file */
	WinOffset offset; /* Offset of current view. Counts tabs as 1 character */
	WinCur cur; /* Pointer to window's content. Counts tabs as 1 character */
	struct winsize size; /* Window size */
};

/* Clamps cursor to the line. Useful when moving between lines. */
static void win_clamp_cur_to_line(Win *const win);

/*
Gets the count of characters by which the part of line is expanded using tabs.
The part of the line from the beginning to the passed column is considered.
*/
static size_t win_exp_col(const Line *const line, const size_t col);

/*
Fixes expanded cursor column for current line. Used than expanded cursor goes
off window, but the internal cursor is still here.
*/
static void win_fix_exp_cur_col(Win *const win);

/* Gets current line. */
static Line *win_get_curr_line(const Win *const win);

/* Gets line by its index. */
static Line *win_get_line(const Win *const win, const size_t idx);

/* Updates window's size using terminal. */
static void win_upd_size(Win *const win);

static void
win_clamp_cur_to_line(Win *const win)
{
	const Line *const line = win_get_curr_line(win);

	/* Check that cursor out of the line */
	if (win->offset.cols + win->cur.col > line->len) {
		/* Check that line not in the window */
		if (line->len <= win->offset.cols) {
			win->offset.cols = 0 == line->len ? 0 : line->len - 1;
			win->cur.col = 0 == line->len ? 0 : 1;
		} else {
			win->cur.col = line->len - win->offset.cols;
		}
	}
}

void
win_close(Win *const win)
{
	/* Deinitialize terminal, close file and deallocate opaque struct */
	term_deinit();
	file_close(win->file);
	free(win);
}

int
win_del_line(Win *const win, size_t times)
{
	size_t lines_cnt = file_lines(win->file)->cnt;

	if (1 >= lines_cnt) {
		return -1;
	} else if (times > 0) {
		/* Get real repeat times */
		times = MIN(times, lines_cnt - win->offset.rows - win->cur.row);

		/* The file must contain at least one line */
		if (times == lines_cnt)
			times--;

		/* Remove column offsets */
		win_mv_to_begin_of_line(win);

		/* Times is never zero here */
		while (times-- > 0)
			/* Delete lines */
			file_del(win->file, win->offset.rows + win->cur.row);

		/* Move up if we deleted the last line and stayed there */
		if (win->offset.rows + win->cur.row == file_lines(win->file)->cnt)
			win_mv_up(win, 1);
	}
	return 0;
}

void
win_draw_cur(const Win *const win, Buf *const buf)
{
	/* Get current line */
	const Line *const line = win_get_curr_line(win);

	/* Expand offset and file columns */
	size_t exp_offset_col = win_exp_col(line, win->offset.cols);
	size_t exp_file_col = win_exp_col(line, win->offset.cols + win->cur.col);

	/* Substract expanded columns to get real column in the window */
	esc_cur_set(buf, win->cur.row, exp_file_col - exp_offset_col);
}

void
win_draw_lines(const Win *const win, Buf *const buf)
{
	size_t exp_offset_col;
	size_t row;
	size_t len_to_draw;
	size_t lines_cnt = file_lines(win->file)->cnt;
	const Line *line;

	for (row = 0; row + STAT_ROWS_CNT < win->size.ws_row; row++) {
		/* Checking if there is a line to draw at this row */
		if (win->offset.rows + row >= lines_cnt) {
			buf_write(buf, "~", 1);
		} else {
			/* Get current line */
			line = win_get_line(win, win->offset.rows + row);

			/* Get expanded with tabs offset's column */
			exp_offset_col = win_exp_col(line, win->offset.cols);

			/* Draw line if not empty and not hidden behind offset */
			if (line->render_len > exp_offset_col) {
				/* Calculate length to draw using expanded length and draw */
				len_to_draw = MIN(win->size.ws_col, line->render_len - exp_offset_col);
				buf_write(buf, &line->render[exp_offset_col], len_to_draw);
			}
		}

		/* Move to the beginning of the next row */
		buf_write(buf, "\r\n", 2);
	}
}

static size_t
win_exp_col(const Line *const line, const size_t col)
{
	size_t i;
	size_t ret;
	size_t end = MIN(col, line->len);

	/* Iterate over every character in the visible part of line */
	for (i = 0, ret = 0; i < end; i++, ret++) {
		/* Expand tabs */
		if ('\t' == line->cont[i])
			ret += CFG_TAB_SIZE - ret % CFG_TAB_SIZE - 1;
	}
	return ret;
}

const File*
win_file(const Win *const win)
{
	return win->file;
}

static void
win_fix_exp_cur_col(Win *const win)
{
	const Line *const line = win_get_curr_line(win);
	/* Shift column offset until we see expanded cursor */
	while (
		win_exp_col(line, win->offset.cols + win->cur.col)
			- win_exp_col(line, win->offset.cols)
				>= win->size.ws_col
	) {
		win->offset.cols++;
		win->cur.col--;
	}
}

static Line*
win_get_curr_line(const Win *const win)
{
	return win_get_line(win, win->offset.rows + win->cur.row);
}

size_t
win_get_curr_line_idx(const Win *const win)
{
	return win->offset.rows + win->cur.row;
}

size_t
win_get_curr_line_cont_idx(const Win *const win)
{
	return win->offset.cols + win->cur.col;
}

static Line*
win_get_line(const Win *const win, const size_t idx)
{
	return file_get(win->file, idx);
}

void
win_handle_signal(Win *const win, const int signal)
{
	if (SIGWINCH == signal)
		win_upd_size(win);
}

void
win_mv_down(Win *const win, size_t times)
{
	size_t lines_cnt = file_lines(win->file)->cnt;

	if (times > 0) {
		while (times-- > 0) {
			/* Break if there is no more space to move down */
			if (win->offset.rows + win->cur.row + 1 >= lines_cnt)
				break;

			/* Check that there is no space in current window */
			if (win->cur.row + STAT_ROWS_CNT + 1 == win->size.ws_row)
				win->offset.rows++;
			else
				win->cur.row++;
		}

		/* Clamp cursor to line after move down several times */
		win_clamp_cur_to_line(win);
	}
}

void
win_mv_left(Win *const win, size_t times)
{
	if (times > 0) {
		while (times-- > 0) {
			/* Move to the beginning of next line if there is not space to move right */
			if (win->offset.cols + win->cur.col == 0) {
				/* Check there is no next line */
				if (win->offset.rows + win->cur.row == 0)
					break;

				/* Move to the end of previous line */
				win_mv_up(win, 1);
				win_mv_to_end_of_line(win);
			} else if (win->cur.col == 0) {
				/* We are at the right of window */
				win->offset.cols--;
			} else {
				/* We are have enough space to move right in the current window */
				win->cur.col--;
			}
		}

		/* Fix expanded cursor column during left movement */
		win_fix_exp_cur_col(win);
	}
}

void
win_mv_right(Win *const win, size_t times)
{
	size_t lines_cnt = file_lines(win->file)->cnt;
	const Line *line = win_get_curr_line(win);

	if (times > 0) {
		while (times-- > 0) {
			/* Move to the beginning of next line if there is not space to move right */
			if (win->offset.cols + win->cur.col == line->len) {
				/* Check there is no next line */
				if (win->offset.rows + win->cur.row + 1 == lines_cnt)
					break;

				/* Move to the beginning of next line */
				win_mv_to_begin_of_line(win);
				win_mv_down(win, 1);
				line++;
			} else if (win->cur.col + 1 == win->size.ws_col) {
				/* We are at the right of window */
				win->offset.cols++;
			} else {
				/* We are have enough space to move right in the current window */
				win->cur.col++;
			}
		}

		/* Fix expanded cursor column during right movement */
		win_fix_exp_cur_col(win);
	}
}

void
win_mv_to_begin_of_file(Win *const win)
{
	/* Move to begin of first line */
	win_mv_to_begin_of_line(win);
	win->offset.rows = 0;
	win->cur.row = 0;
}

void
win_mv_to_begin_of_line(Win *const win)
{
	/* Zeroize column positions */
	win->offset.cols = 0;
	win->cur.col = 0;
}

void
win_mv_to_end_of_file(Win *const win)
{
	size_t lines_cnt = file_lines(win->file)->cnt;
	/* Move to begin of last line */
	win_mv_to_begin_of_line(win);

	/* Check that line on initial frame */
	if (lines_cnt < win->size.ws_row) {
		win->offset.rows = 0;
		win->cur.row = lines_cnt - 1;
	} else {
		win->offset.rows = lines_cnt - (win->size.ws_row - STAT_ROWS_CNT);
		win->cur.row = win->size.ws_row - STAT_ROWS_CNT - 1;
	}
}

void
win_mv_to_end_of_line(Win *const win)
{
	const Line *const line = win_get_curr_line(win);

	/* Check that end of line in the current window */
	if (line->len < win->offset.cols + win->size.ws_col) {
		win->cur.col = line->len - win->offset.cols;
	} else {
		win->offset.cols = line->len - win->size.ws_col + 1;
		win->cur.col = win->size.ws_col - 1;
	}
}

void
win_mv_to_next_word(Win *const win, size_t times)
{
	size_t cont_i;
	size_t word_i;
	const Line *const line = win_get_curr_line(win);

	if (times > 0) {
		while (times-- > 0) {
			/* Find next word from current position until end of line */
			cont_i = win->offset.cols + win->cur.col;
			word_i = word_next(&line->cont[cont_i], line->len - cont_i);

			/* Check that word in the current window */
			if (win->cur.col + word_i < win->size.ws_col) {
				win->cur.col += word_i;
			} else {
				win->offset.cols = cont_i + word_i - win->size.ws_col + 1;
				win->cur.col = win->size.ws_col - 1;
			}

			/* Check that we at end of line */
			if (cont_i + word_i == line->len)
				break;
		}

		/* Fix expanded cursor column during right movement */
		win_fix_exp_cur_col(win);
	}
}

void
win_mv_to_prev_word(Win *const win, size_t times)
{
	size_t cont_i;
	size_t word_i;
	const Line *const line = win_get_curr_line(win);

	if (times > 0) {
		while (times-- > 0) {
			/* Find next word from current position until start of line */
			cont_i = win->offset.cols + win->cur.col;
			word_i = word_rnext(line->cont, cont_i);

			/* Check that word in the current window */
			if (word_i >= win->offset.cols) {
				win->cur.col = word_i - win->offset.cols;
			} else {
				win->offset.cols = word_i - 1;
				win->cur.col = 1;
			}

			/* Check that we at start of line */
			if (word_i == 0)
				break;
		}

		/* Fix expanded cursor column during left movement */
		win_fix_exp_cur_col(win);
	}
}

void
win_mv_up(Win *const win, size_t times)
{
	if (times > 0) {
		while (times-- > 0) {
			/* Break if there is no more space to move down */
			if (win->offset.rows + win->cur.row == 0)
				break;

			/* Check that there is no space in current window */
			if (0 == win->cur.row)
				win->offset.rows--;
			else
				win->cur.row--;
		}

		/* Clamp cursor to line after move down several times */
		win_clamp_cur_to_line(win);
	}
}

Win*
win_open(const char *const path, const int ifd, const int ofd)
{
	/* Allocate window */
	Win *const win = malloc(sizeof(*win));
	if (NULL == win)
		err(EXIT_FAILURE, "Failed to allocate window");

	/* Open file */
	win->file = file_open(path);
	/* Initialize offset and cursor */
	memset(&win->offset, 0, sizeof(win->offset));
	memset(&win->cur, 0, sizeof(win->cur));

	/* Initialize terminal with accepted descriptors */
	term_init(ifd, ofd);
	/* Get window size */
	term_get_win_size(&win->size);
	return win;
}

size_t
win_save_file(Win *const win)
{
	return file_save(win->file, NULL);
}

size_t
win_save_file_to_spare_dir(Win *const win, char *const  path, size_t len)
{
	return file_save_to_spare_dir(win->file, path, len);
}

struct winsize
win_size(const Win *const win)
{
	return win->size;
}

static void
win_upd_size(Win *const win)
{
	/* Update size using terminal */
	term_get_win_size(&win->size);
	/* Clamp cursor to window */
	win->cur.row = MIN(win->cur.row, win->size.ws_row - STAT_ROWS_CNT - 1);
	win->cur.col = MIN(win->cur.col, win->size.ws_col - 1);

	/* Clamp cursor to new row */
	win_clamp_cur_to_line(win);
}
