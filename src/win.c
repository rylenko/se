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
Draws row on the window if exists or special config string.

Returns 0 on success and -1 on error.
*/
static int win_draw_line(const struct Win *, Vec *, unsigned short);

/*
Gets the count of characters by which the part of line is expanded using tabs.
The part of the line from the beginning to the passed column is considered.
*/
static size_t win_exp_col(const char *, size_t, size_t);

/*
Collection of methods to scroll and fix cursor.

Returns 0 on success and -1 on error.
*/
static int win_scroll(struct Win *);

int
win_close(struct Win *const win)
{
	int ret;

	/* Deinitialize the terminal */
	ret = term_deinit();
	if (-1 == ret)
		return -1;

	/* Close opened file */
	file_close(win->file);
	/* Free opaque struct */
	free(win);
	return 0;
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

int
win_break_line(struct Win *const win)
{
	int ret;
	const size_t idx = win_curr_line_idx(win);
	const size_t pos = win_curr_line_char_idx(win);

	/* Break current line at current cursor's position */
	ret = file_break_line(win->file, idx, pos);
	if (-1 == ret)
		return -1;

	/* Move to the beginning of the next line */
	win_mv_to_begin_of_line(win);
	ret = win_mv_down(win, 1);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_del_char(struct Win *const win)
{
	int ret;
	const size_t line_idx = win_curr_line_idx(win);
	const size_t char_idx = win_curr_line_char_idx(win);

	/* Check that we are not at the beginning of the line */
	if (char_idx > 0) {
		/* Delete character */
		ret = file_del_char(win->file, line_idx, char_idx - 1);
		if (-1 == ret)
			return -1;

		/* Move left after deleting */
		ret = win_mv_left(win, 1);
		if (-1 == ret)
			return -1;
	} else if (line_idx > 0) {
		/* We are at the beginning of not first line. Move to end of previous line */
		ret = win_mv_left(win, 1);
		if (-1 == ret)
			return -1;

		/* Absorb current line to previous line */
		ret = file_absorb_next_line(win->file, line_idx);
		if (-1 == ret)
			return -1;
	}

	/* Fix expanded cursor column */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_del_line(struct Win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	/* Get real repeat times */
	times = MIN(times, file_lines_cnt(win->file) - win_curr_line_idx(win));

	/* Remove column offsets */
	win_mv_to_begin_of_line(win);

	while (times-- > 0) {
		/* Delete line */
		ret = file_del_line(win->file, win_curr_line_idx(win));
		if (-1 == ret)
			return -1;

		/* Move up if we deleted the last line and cursor stayed there */
		if (win_curr_line_idx(win) == file_lines_cnt(win->file)) {
			ret = win_mv_up(win, 1);
			if (-1 == ret)
				return -1;
		}
	}
	return 0;
}

int
win_draw_cur(const struct Win *const win, Vec *const vec)
{
	int ret;
	const char *chars;
	size_t len;
	size_t exp_offset_col;
	size_t exp_col;

	/* Get line characters */
	chars = file_line_chars(win->file, win_curr_line_idx(win));
	if (NULL == chars)
		return -1;

	/* Get line length */
	ret = file_line_len(win->file, win_curr_line_idx(win), &len);
	if (-1 == ret)
		return -1;

	/* Expand offset and file columns */
	exp_offset_col = win_exp_col(chars, len, win->offset.cols);
	exp_f_col = win_exp_col(chars, len, win->offset.cols + win->cur.col);

	/* Sub expanded columns to get real column in the window and set cursor */
	esc_cur_set(vec, win->cur.row, exp_col - exp_offset_col);
}

static int
win_draw_line(
	const struct Win *const win,
	Vec *const buf,
	const unsigned short row
) {
	int ret;
	const char *chars;
	size_t len;
	const char *render;
	size_t render_len;
	size_t exp_offset_col;
	size_t len_to_draw;
	const size_t lines_cnt = file_lines_cnt(win->file);

	/* Checking if there is a line to draw at this row */
	if (win->offset.rows + row >= lines_cnt) {
		ret = vec_append(buf, &cfg_no_line, 1);
		if (-1 == ret)
			return -1;
	} else {
		/* Get current line details */
		chars = file_line_chars(win->file, win->offset.rows + row);
		len = file_line_len(win->file, win->offset.rows + row);
		render = file_line_render(win->file, win->offset.rows + row);
		render_len = file_line_render_len(win->file, win->offset.rows + row);

		/* Get expanded with tabs offset's column */
		exp_offset_col = win_exp_col(chars, len, win->offset.cols);
		/* Do nothing if line hidden behind offset or empty */
		if (render_len <= exp_offset_col) {
			return 0;
		}

		/* Calculate length to draw using expanded length and draw */
		len_to_draw = MIN(win->size.ws_col, render_len - exp_offset_col);
		ret = vec_append(buf, &render[exp_offset_col], len_to_draw);
		if (-1 == ret)
			return -1;
	}
	return 0;
}

int
win_draw_lines(const struct Win *const win, Vec *const buf)
{
	int ret;
	unsigned short row;

	/* Set colors */
	ret = esc_color_begin(buf, &cfg_color_lines_fg, NULL);
	if (-1 == ret)
		return -1;

	for (row = 0; row + STAT_ROWS_CNT < win->size.ws_row; row++) {
		/* Draw line */
		ret = win_draw_line(win, buf, row);
		if (-1 == ret)
			return -1;

		/* Move to the beginning of the next row */
		ret = vec_append(buf, "\r\n", 2);
		if (-1 == ret)
			return -1;
	}

	/* End colored output */
	ret = esc_color_end(buf);
	if (-1 == ret)
		return -1;
	return 0;
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

int
win_ins_char(struct Win *const win, const char ch)
{
	int ret;
	const size_t line_idx = win_curr_line_idx(win);
	const size_t char_idx = win_curr_line_char_idx(win);

	/* Insert character */
	ret = file_ins_char(win->file, line_idx, char_idx, ch);
	if (-1 == ret)
		return -1;

	/* Move right after insertion */
	ret = win_mv_right(win, 1);
	if (-1 == ret)
		return -1;

	/* Fix expanded cursor column */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_ins_empty_line_below(struct Win *const win, const size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	/* Remove column offsets */
	win_mv_to_begin_of_line(win);

	/* Insert empty lines */
	while (times-- > 0)
		ret = file_ins_empty_line(win->file, win_curr_line_idx(win) + 1);
		if (-1 == ret)
			return -1;
	}

	/* Move to last inserted line */
	win_mv_down(win, times);
	return 0;
}

int
win_ins_empty_line_on_top(struct Win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	/* Reove column offsets */
	win_mv_to_begin_of_line(win);

	/* Insert empty lines */
	while (times-- > 0) {
		ret = file_ins_empty_line(win->file, win_curr_line_idx(win));
		if (-1 == ret)
			return -1;
	}
	return 0;
}

int
win_mv_down(struct Win *const win, size_t times)
{
	int ret;
	size_t lines_cnt = file_lines_cnt(win->file);

	if (0 == times)
		return 0;

	while (times-- > 0) {
		/* Return if there is no more space to move down */
		if (win->offset.rows + win->cur.row + 1 >= lines_cnt)
			return 0;

		/* Check that there is no space in current window */
		if (win->cur.row + STAT_ROWS_CNT + 1 == win->size.ws_row)
			win->offset.rows++;
		else
			win->cur.row++;
	}

	/* Clamp cursor to line after move down several times */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_mv_left(struct Win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	while (times-- > 0) {
		if (win->offset.cols + win->cur.col == 0) {
			/* Check there is no next line */
			if (win->offset.rows + win->cur.row == 0)
				break;

			/* Move to the beginning of next line if no space to move left */
			ret = win_mv_up(win, 1);
			if (-1 == ret)
				return -1;
			win_mv_to_end_of_line(win);
		} else if (win->cur.col == 0) {
			/* We are at the right of window, but have non-zero offset */
			win->offset.cols--;
		} else {
			/* We are have enough space to move right in the current window */
			win->cur.col--;
		}
	}

	/* Fix expanded cursor column during left movement */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_mv_right(struct Win *const win, size_t times)
{
	const size_t lines_cnt = file_lines_cnt(win->file);
	size_t line_len;

	if (0 == times)
		return 0;

	while (times-- > 0) {
		/* Get line length */
		int ret = file_line_len(win->file, win_curr_line_idx(win), &line_len);
		if (-1 == ret)
			return -1;

		if (win_curr_line_char_idx() >= line_len) {
			/* Check there is no next line */
			if (win_curr_line_idx() + 1 == lines_cnt)
				break;

			/* Move to the beginning of next line if no space to move right */
			win_mv_to_begin_of_line(win);
			ret = win_mv_down(win, 1);
			if (-1 == ret)
				return -1;
		} else if (win->cur.col + 1 >= win->size.ws_col) {
			/* We are at the right of window but can shift offset */
			win->offset.cols++;
		} else {
			/* We are have enough space to move right in the current window */
			win->cur.col++;
		}
	}

	/* Fix expanded cursor column during right movement */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
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

int
win_mv_to_end_of_line(struct Win *const win)
{
	const size_t line_idx = win_curr_line_idx(win);
	size_t render_len;
	int ret = file_line_render_len(win->file, line_idx, &render_len);

	/* Check line's render len getting error */
	if (-1 == ret)
		return -1;

	/* Check that end of line in the current window */
	if (render_len < win->offset.cols + win->size.ws_col) {
		win->cur.col = render_len - win->offset.cols;
	} else {
		win->offset.cols = render_len - win->size.ws_col + 1;
		win->cur.col = win->size.ws_col - 1;
	}

	/* Scroll after moving right */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_mv_to_next_word(struct Win *const win, size_t times)
{
	size_t char_idx;
	size_t word_idx;
	const char *const chars = file_line_chars(win->file, win_curr_line_idx(win));
	size_t len;
	int ret = file_line_len(win->file, win_curr_line_idx(win), &len);

	/* Check current line's chars getting error */
	if (NULL == chars)
		return -1;

	/* Check current line's length getting error */
	if (-1 == ret)
		return -1;

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
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_mv_to_prev_word(struct Win *const win, size_t times)
{
	int ret;
	size_t word_i;
	const char *const chars = file_line_chars(win->file, win_curr_line_idx(win));

	if (0 == times)
		return 0;

	/* Check current line's chars getting error */
	if (NULL == chars)
		return -1;

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
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

int
win_mv_up(struct Win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

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
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}

struct Win*
win_open(const char *const path, const int ifd, const int ofd)
{
	int ret;
	struct Win *const win = malloc(sizeof(*win));

	/* Check opaque struct allocation error */
	if (NULL == win)
		return NULL;

	/* Open file */
	win->file = file_open(path);
	if (NULL == win->file)
		goto err_free_opaque;

	/* Initialize offset and cursor */
	memset(&win->offset, 0, sizeof(win->offset));
	memset(&win->cur, 0, sizeof(win->cur));

	/* Initialize terminal with accepted descriptors */
	ret = term_init(ifd, ofd);
	if (-1 == ret)
		goto err_free_opaque_and_close;

	/* Get window size */
	ret = term_get_win_size(&win->size);
	if (-1 == ret)
		goto err_clean_all;
	return win;
err_clean_all:
	/* Errors checking here is useless */
	term_deinit();
err_free_opaque_and_close:
	file_close(win->file);
err_free_opaque:
	free(win);
	return NULL;
}

size_t
win_save_file(struct Win *const win)
{
	const size_t len = file_save(win->file, NULL);
	return len;
}

size_t
win_save_file_to_spare_dir(struct Win *const win, char *const path, size_t len)
{
	const size_t len = file_save_to_spare_dir(win->file, path, len);
	return len;
}

static int
win_scroll(struct Win *const win)
{
	int ret;
	const char *chars;
	size_t line_len;
	size_t line_idx;
	size_t exp_offset_col;
	size_t exp_col;

	/* Scroll to overflowed cursor position. Useful after window resizing */
	if (win->cur.row + STAT_ROWS_CNT >= win->size.ws_row) {
		win->offset.rows += win->cur.row + STAT_ROWS_CNT + 1 - win->size.ws_row;
		win->cur.row = win->size.ws_row - STAT_ROWS_CNT - 1;
	}
	if (win->cur.col >= win->size.ws_col) {
		win->offset.cols += win->cur.col + 1 - win->size.ws_col;
		win->cur.col = win->size.ws_col - 1;
	}

	/* Get current line index */
	line_idx = win_curr_line_idx(win);

	/* Get current line length */
	ret = file_line_len(win->file, line_idx, &line_len);
	if (-1 == ret)
		return -1;
	/* Get current line's chars */
	chars = file_line_chars(win->file, line_idx);
	if (NULL == chars)
		return -1;

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

	/*
	Shift column offset until we see expanded cursor. Useful after moving between
	lines with tabs
	*/
	while (1) {
		/* Get expanded hidden column and expanded viewed column */
		exp_col = win_exp_col(chars, line_len, win->offset.cols + win->cur.col)
		exp_offset_col = win_exp_col(chars, line_len, win->offset.cols);

		/* Check that diff between expansion is not greater than window's width */
		if (exp_col - exp_offset_col < win->size.ws_col)
			break;

		/* TODO: can we shift here only by offset? */
		/* Shift to view pointed content */
		win->offset.cols++;
		win->cur.col--;
	}
	return 0;
}

int
win_search(struct Win *const win, const char *const query, const enum Dir dir)
{
	int ret;
	size_t idx;
	size_t pos;

	if (DIR_FWD == dir) {
		/* Move forward to not collide with previous result */
		ret = win_mv_right(win, 1);
		if (-1 == ret)
			return -1;
	}

	/* Prepare indexes */
	idx = win_curr_line_idx(win);
	pos = win_curr_line_char_idx(win);

	/* Search with accepted query */
	ret = file_search(win->file, &idx, &pos, query, dir)
	if (-1 == ret)
		return -1;
	if (1 == ret) {
		/* Move to begin of line to easily move right to result later */
		win_mv_to_begin_of_line(win);

		/* Move to result's line */
		if (DIR_BWD == dir)
			ret = win_mv_up(win, win_curr_line_idx(win) - idx);
		else if (DIR_FWD == dir)
			ret = win_mv_down(win, idx - win_curr_line_idx(win));
		if (-1 == ret)
			return -1;

		/* Move to result on current line */
		ret = win_mv_right(win, pos);
		if (-1 == ret)
			return -1;
	} else if (DIR_FWD == dir) {
		/* Move back to start position if no results during forward searching */
		ret = win_mv_left(win, 1);
		if (-1 == ret)
			return -1;
	}
	return 0;
}

struct winsize
win_size(const struct Win *const win)
{
	return win->size;
}

int
win_upd_size(struct Win *const win)
{
	int ret;

	/* Update size using terminal */
	ret = term_get_win_size(&win->size);
	if (-1 == ret)
		return -1;

	/* Scroll after resize */
	ret = win_scroll(win);
	if (-1 == ret)
		return -1;
	return 0;
}
