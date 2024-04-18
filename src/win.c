#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "alloc.h"
#include "cfg.h"
#include "esc.h"
#include "file.h"
#include "math.h"
#include "term.h"
#include "vec.h"
#include "win.h"
#include "word.h"

enum {
	STAT_ROWS_CNT = 1, /* Count of rows reserved for status */
};

/* Cursor's poisition in the window. */
struct Cur {
	unsigned short row;
	unsigned short col;
};

/* Offset in the file for current view. */
struct Offset {
	size_t rows;
	size_t cols;
};

/*
Window parameters.

Position parameters may differ from real ones due to tabs expansion.
*/
struct Win {
	File *file; /* Opened file */
	struct Offset offset; /* Offset of view/file. Tab's width is 1 */
	struct Cur cur; /* Pointer to the viewed char. Tab's width is 1 */
	struct winsize size; /* Terminal window size */
};

/*
Gets the count of characters by which the part of line is expanded using tabs.
The part of the line from the beginning to the passed column is considered.
*/
static size_t win_exp_col(const char *, size_t, size_t);

/* Collection of methods to scroll and fix cursor. */
static void win_scroll(struct Win *);

void
win_close(struct Win *const win)
{
	/* Deinitialize terminal, close file and deallocate opaque struct */
	term_deinit();
	file_close(win->file);
	free(win);
}

size_t
win_curr_line_idx(const struct Win *const win)
{
	return win->offset.rows + win->cur.row;
}

size_t
win_curr_line_char_idx(const struct Win *const win)
{
	return win->offset.cols + win->cur.col;
}

void
win_break_line(struct Win *const win)
{
	const size_t idx = win_curr_line_idx(win);
	const size_t pos = win_curr_line_char_idx(win);
	/* Break current line at current cursor's position */
	file_break_line(win->file, idx, pos);
	/* Move to the beginning of the next line */
	win_mv_to_begin_of_line(win);
	win_mv_down(win, 1);
}

void
win_del_char(struct Win *const win)
{
	const size_t char_idx = win_curr_line_char_idx(win);

	/* Check that we are not at the beginning of the line */
	if (char_idx > 0) {
		/* Delete character */
		file_del_char(win->file, win_curr_line_idx(win), char_idx - 1);
		/* Move left after deleting */
		win_mv_left(win, 1);
	} else if (win_curr_line_idx(win) > 0) {
		/* Absorb current line to next line if we are at the beginning of the line */
		win_mv_left(win, 1);
		file_absorb_next_line(win->file, win_curr_line_idx(win));
	}

	/* Fix expanded cursor column */
	win_scroll(win);
}

int
win_del_line(struct Win *const win, size_t times)
{
	/* Remember that file must contain at least one line */
	size_t lines_cnt = file_lines_cnt(win->file);
	if (lines_cnt <= 1) {
		return -1;
	}

	if (times > 0) {
		/* Get real repeat times */
		times = MIN(times, lines_cnt - win->offset.rows - win->cur.row);
		/* The file must contain at least one line */
		if (times == lines_cnt)
			times--;

		/* Remove column offsets */
		win_mv_to_begin_of_line(win);

		/* Delete lines */
		while (times-- > 0)
			file_del_line(win->file, win->offset.rows + win->cur.row);

		/* Move up if we deleted the last line and stayed there */
		if (win->offset.rows + win->cur.row == file_lines_cnt(win->file))
			win_mv_up(win, 1);
	}
	return 0;
}

void
win_draw_cur(const struct Win *const win, Vec *const vec)
{
	/* Get current line */
	const char *const chars = file_line_chars(win->file, win_curr_line_idx(win));
	const size_t len = file_line_len(win->file, win_curr_line_idx(win));
	/* Expand offset and file columns */
	size_t exp_offset_col = win_exp_col(chars, len, win->offset.cols);
	size_t exp_f_col = win_exp_col(chars, len, win->offset.cols + win->cur.col);

	/* Substract expanded columns to get real column in the window */
	esc_cur_set(vec, win->cur.row, exp_f_col - exp_offset_col);
}

void
win_draw_lines(const struct Win *const win, Vec *const buf)
{
	const char *chars;
	size_t exp_offset_col;
	size_t row;
	size_t len;
	size_t len_to_draw;
	size_t lines_cnt = file_lines_cnt(win->file);
	size_t render_len;
	const char *render;

	/* Set colors */
	esc_color_begin(buf, &cfg_color_lines_fg, NULL);

	for (row = 0; row + STAT_ROWS_CNT < win->size.ws_row; row++) {
		/* Checking if there is a line to draw at this row */
		if (win->offset.rows + row >= lines_cnt) {
			vec_append(buf, "~", 1);
		} else {
			/* Get current line details */
			chars = file_line_chars(win->file, win->offset.rows + row);
			len = file_line_len(win->file, win->offset.rows + row);
			render = file_line_render(win->file, win->offset.rows + row);
			render_len = file_line_render_len(win->file, win->offset.rows + row);

			/* Get expanded with tabs offset's column */
			exp_offset_col = win_exp_col(chars, len, win->offset.cols);

			/* Draw line if not empty and not hidden behind offset */
			if (render_len > exp_offset_col) {
				/* Calculate length to draw using expanded length and draw */
				len_to_draw = MIN(win->size.ws_col, render_len - exp_offset_col);
				vec_append(buf, &render[exp_offset_col], len_to_draw);
			}
		}

		/* Move to the beginning of the next row */
		vec_append(buf, "\r\n", 2);
	}

	/* End colored output */
	esc_color_end(buf);
}

static size_t
win_exp_col(const char *const chars, const size_t len, const size_t col)
{
	size_t i;
	size_t ret;
	size_t end = MIN(col, len);

	/* Iterate over every character in the visible part of line */
	for (i = 0, ret = 0; i < end; i++, ret++) {
		/* Expand tabs */
		if ('\t' == chars[i])
			ret += CFG_TAB_SIZE - ret % CFG_TAB_SIZE - 1;
	}
	return ret;
}

char
win_file_is_dirty(const struct Win *const win)
{
	return file_is_dirty(win->file);
}

const char*
win_file_path(const struct Win *const win)
{
	return file_path(win->file);
}

static void
win_scroll(struct Win *const win)
{
	const char *chars;
	size_t line_len;

	/* Scroll to overflowed cursor position. Useful after window resizing */
	if (win->cur.row + STAT_ROWS_CNT >= win->size.ws_row) {
		win->offset.rows += win->cur.row + STAT_ROWS_CNT + 1 - win->size.ws_row;
		win->cur.row = win->size.ws_row - STAT_ROWS_CNT - 1;
	}
	if (win->cur.col >= win->size.ws_col) {
		win->offset.cols += win->cur.col + 1 - win->size.ws_col;
		win->cur.col = win->size.ws_col - 1;
	}

	/* Get current line's length */
	line_len = file_line_len(win->file, win_curr_line_idx(win));

	/* Check that cursor out of the line. Useful after move down and up */
	if (win->offset.cols + win->cur.col > line_len) {
		/* Check that line not in the window */
		if (line_len <= win->offset.cols) {
			win->offset.cols = 0 == line_len ? 0 : line_len - 1;
			win->cur.col = 0 == line_len ? 0 : 1;
		} else {
			win->cur.col = line_len - win->offset.cols;
		}
	}

	/* Get current line's chars */
	chars = file_line_chars(win->file, win_curr_line_idx(win));

	/*
	Shift column offset until we see expanded cursor. Useful after moving between
	lines with tabs
	*/
	while (
		win_exp_col(chars, line_len, win->offset.cols + win->cur.col)
			- win_exp_col(chars, line_len, win->offset.cols)
				>= win->size.ws_col
	) {
		win->offset.cols++;
		win->cur.col--;
	}
}

void
win_ins_char(struct Win *const win, const char ch)
{
	/* Insert character and mark file as dirty */
	const size_t idx = win_curr_line_idx(win);
	const size_t pos = win_curr_line_char_idx(win);
	file_ins_char(win->file, idx, pos, ch);

	/* Move right after insertion */
	win_mv_right(win, 1);

	/* Fix expanded cursor column */
	win_scroll(win);
}

void
win_ins_empty_line_below(struct Win *const win, const size_t times)
{
	size_t times_i = times;

	if (times > 0) {
		/* Remove column offsets */
		win_mv_to_begin_of_line(win);
		/* Insert empty lines */
		while (times_i-- > 0)
			file_ins_empty_line(win->file, win_curr_line_idx(win) + 1);
		/* Move to last inserted line */
		win_mv_down(win, times);
	}
}

void
win_ins_empty_line_on_top(struct Win *const win, size_t times)
{
	if (times > 0) {
		/* Reove column offsets */
		win_mv_to_begin_of_line(win);
		/* Insert empty lines */
		while (times-- > 0)
			file_ins_empty_line(win->file, win_curr_line_idx(win));
	}
}

void
win_mv_down(struct Win *const win, size_t times)
{
	size_t lines_cnt = file_lines_cnt(win->file);

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
	win_scroll(win);
}

void
win_mv_left(struct Win *const win, size_t times)
{
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
	win_scroll(win);
}

void
win_mv_right(struct Win *const win, size_t times)
{
	const size_t lines_cnt = file_lines_cnt(win->file);
	size_t line_len = file_line_len(win->file, win_curr_line_idx(win));

	while (times-- > 0) {
		/* Move to the beginning of next line if there is not space to move right */
		if (win->offset.cols + win->cur.col == line_len) {
			/* Check there is no next line */
			if (win->offset.rows + win->cur.row + 1 == lines_cnt)
				break;

			/* Move to the beginning of next line */
			win_mv_to_begin_of_line(win);
			win_mv_down(win, 1);
			line_len = file_line_len(win->file, win_curr_line_idx(win));
		} else if (win->cur.col + 1 == win->size.ws_col) {
			/* We are at the right of window */
			win->offset.cols++;
		} else {
			/* We are have enough space to move right in the current window */
			win->cur.col++;
		}
	}

	/* Fix expanded cursor column during right movement */
	win_scroll(win);
}

void
win_mv_to_begin_of_file(struct Win *const win)
{
	/* Move to begin of first line */
	win_mv_to_begin_of_line(win);
	win->offset.rows = 0;
	win->cur.row = 0;
}

void
win_mv_to_begin_of_line(struct Win *const win)
{
	/* Zeroize column positions */
	win->offset.cols = 0;
	win->cur.col = 0;
}

void
win_mv_to_end_of_file(struct Win *const win)
{
	const size_t lines_cnt = file_lines_cnt(win->file);
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
win_mv_to_end_of_line(struct Win *const win)
{
	const size_t render_len = \
		file_line_render_len(win->file, win_curr_line_idx(win));

	/* Check that end of line in the current window */
	if (render_len < win->offset.cols + win->size.ws_col) {
		win->cur.col = render_len - win->offset.cols;
	} else {
		win->offset.cols = render_len - win->size.ws_col + 1;
		win->cur.col = win->size.ws_col - 1;
	}

	win_scroll(win);
}

void
win_mv_to_next_word(struct Win *const win, size_t times)
{
	size_t char_idx;
	size_t word_idx;
	const char *const chars = file_line_chars(win->file, win_curr_line_idx(win));
	const size_t len = file_line_len(win->file, win_curr_line_idx(win));

	while (times-- > 0) {
		/* Find next word from current position until end of line */
		char_idx = win_curr_line_char_idx(win);
		word_idx = word_next(&chars[char_idx], len - char_idx);

		/* Check that word in the current window */
		if (win->cur.col + word_idx < win->size.ws_col) {
			win->cur.col += word_idx;
		} else {
			win->offset.cols = char_idx + word_idx - win->size.ws_col + 1;
			win->cur.col = win->size.ws_col - 1;
		}

		/* Check that we at end of line */
		if (char_idx + word_idx == len)
			break;
	}

	/* Fix expanded cursor column during right movement */
	win_scroll(win);
}

void
win_mv_to_prev_word(struct Win *const win, size_t times)
{
	size_t word_i;
	const char *const chars = file_line_chars(win->file, win_curr_line_idx(win));

	while (times-- > 0) {
		/* Find next word from current position until start of line */
		word_i = word_rnext(chars, win_curr_line_char_idx(win));

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
	win_scroll(win);
}

void
win_mv_up(struct Win *const win, size_t times)
{
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
	win_scroll(win);
}

struct Win*
win_open(const char *const path, const int ifd, const int ofd)
{
	/* Allocate window */
	struct Win *const win = malloc_err(sizeof(*win));

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
win_save_file(struct Win *const win)
{
	return file_save(win->file, NULL);
}

size_t
win_save_file_to_spare_dir(struct Win *const win, char *const path, size_t len)
{
	return file_save_to_spare_dir(win->file, path, len);
}

void
win_search(struct Win *const win, const char *const query, const enum Dir dir)
{
	size_t idx;
	size_t pos;

	if (DIR_FWD == dir) {
		/* Move forward to not collide with previous result */
		win_mv_right(win, 1);
	}

	/* Prepare indexes */
	idx = win_curr_line_idx(win);
	pos = win_curr_line_char_idx(win);

	/* Search with accepted query */
	if (file_search(win->file, &idx, &pos, query, dir)) {
		/* Move to result */
		win_mv_to_begin_of_line(win);
		if (DIR_BWD == dir)
			win_mv_up(win, win_curr_line_idx(win) - idx);
		else if (DIR_FWD == dir)
			win_mv_down(win, idx - win_curr_line_idx(win));
		win_mv_right(win, pos);
	} else if (DIR_FWD == dir) {
		/* Move back to start position if no results */
		win_mv_left(win, 1);
	}
}

struct winsize
win_size(const struct Win *const win)
{
	return win->size;
}

void
win_upd_size(struct Win *const win)
{
	/* Update size using terminal and fix cursor */
	term_get_win_size(&win->size);
	win_scroll(win);
}
