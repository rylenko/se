#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "cfg.h"
#include "esc.h"
#include "file.h"
#include "math.h"
#include "str.h"
#include "term.h"
#include "vec.h"
#include "win.h"
#include "word.h"

enum {
	STAT_ROWS_CNT = 1, /* Count of rows reserved for status. */
};

/*
 * Cursor's poisition in the window.
 */
struct cur {
	unsigned short row;
	unsigned short col;
};

/*
 * Offset in the file for current view.
 */
struct offset {
	size_t rows;
	size_t cols;
};

/*
 * Window parameters.
 *
 * Position parameters may differ from real ones due to tabs expansion.
 */
struct win {
	struct file *file; /* Opened file. */
	struct offset offset; /* offset of view/file. Tab's width is 1. */
	struct cur cur; /* Pointer to the viewed char. Tab's width is 1. */
	struct winsize size; /* Terminal window size. */
};

/*
 * Draws row on the window if exists or special config string.
 *
 * Returns 0 on success and -1 on error.
 */
static int win_draw_line(const struct win *, struct vec *, unsigned short);

/*
 * Gets the count of characters by which the part of line is expanded using
 * tabs. The part of the line from the beginning to the passed column is
 * considered.
 */
static size_t win_exp_col(const struct pub_line *, size_t);

/*
 * Collection of methods to scroll and fix cursor.
 *
 * Returns 0 on success and -1 on error.
 */
static int win_scroll(struct win *);

/*
 * Scrolls column offset until we see expanded cursor. Useful after moving
 * between lines with tabs.
 */
static int win_scroll_exp_col(struct win *);

/*
 * Scrolls overflowed cursor position back on the screen. Useful after window
 * resizing.
 */
static void win_scroll_overflowed_cur(struct win *);

/*
 * Scrolls cursor back to the line. Useful after move down and up.
 */
static int win_scroll_to_line(struct win *);

int
win_close(struct win *const win)
{
	int ret;

	/* Deinitialize the terminal. */
	ret = term_deinit();
	if (-1 == ret)
		return -1;

	/* Close opened file. */
	file_close(win->file);
	/* Free opaque struct. */
	free(win);
	return 0;
}

size_t
win_curr_line_idx(const struct win *const win)
{
	return win->offset.rows + win->cur.row;
}

size_t
win_curr_line_char_idx(const struct win *const win)
{
	return win->offset.cols + win->cur.col;
}

int
win_break_line(struct win *const win)
{
	int ret;

	/* Break current line at current cursor's position. */
	ret = file_break_line(
		win->file,
		win_curr_line_idx(win),
		win_curr_line_char_idx(win)
	);
	if (-1 == ret)
		return -1;

	/* Move to the beginning of the next line. */
	win_mv_to_begin_of_line(win);
	ret = win_mv_down(win, 1);
	return ret;
}

int
win_del_char(struct win *const win)
{
	int ret;

	/* Check that we are not at the beginning of the line. */
	if (win_curr_line_char_idx(win) > 0) {
		/* Delete character. */
		ret = file_del_char(
			win->file,
			win_curr_line_idx(win),
			win_curr_line_char_idx(win) - 1
		);
		if (-1 == ret)
			return -1;

		/* Move left after deleting. */
		ret = win_mv_left(win, 1);
		if (-1 == ret)
			return -1;
	} else if (win_curr_line_idx(win) > 0) {
		/*
		 * We are at the beginning of not first line. Move to end of previous line.
		 */
		ret = win_mv_left(win, 1);
		if (-1 == ret)
			return -1;

		/* Absorb current line to previous line. */
		ret = file_absorb_next_line(win->file, win_curr_line_idx(win));
		if (-1 == ret)
			return -1;
	}

	/* Fix expanded cursor column. */
	ret = win_scroll(win);
	return ret;
}

int
win_del_line(struct win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	/* Get real repeat times. */
	times = MIN(times, file_lines_cnt(win->file) - win_curr_line_idx(win));

	/* Remove column offsets. */
	win_mv_to_begin_of_line(win);

	while (times-- > 0) {
		/* Delete line. */
		ret = file_del_line(win->file, win_curr_line_idx(win));
		if (-1 == ret)
			return -1;

		/* Move up if we deleted the last line and cursor stayed there. */
		if (win_curr_line_idx(win) == file_lines_cnt(win->file)) {
			ret = win_mv_up(win, 1);
			if (-1 == ret)
				return -1;
		}
	}
	return 0;
}

int
win_draw_cur(const struct win *const win, struct vec *const buf)
{
	int ret;
	struct pub_line line;
	size_t exp_offset_col;
	size_t exp_col;

	/* Get line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	/* Expand offset and file columns. */
	exp_offset_col = win_exp_col(&line, win->offset.cols);
	exp_col = win_exp_col(&line, win->offset.cols + win->cur.col);

	/* Sub expanded columns to get real column in the window and set cursor. */
	esc_cur_set(buf, win->cur.row, exp_col - exp_offset_col);
	return 0;
}

static int
win_draw_line(
	const struct win *const win, struct vec *const buf, const unsigned short row)
{
	int ret;
	struct pub_line line;
	size_t exp_offset_col;
	size_t len_to_draw;
	size_t lines_cnt;

	/* Get lines count. */
	lines_cnt = file_lines_cnt(win->file);

	/* Checking if there is a line to draw at this row. */
	if (win->offset.rows + row >= lines_cnt) {
		ret = vec_append(buf, &cfg_no_line, 1);
		return ret;
	}

	/* Get line. */
	ret = file_line(win->file, win->offset.rows + row, &line);
	if (-1 == ret)
		return -1;

	/* Get expanded with tabs offset's column. */
	exp_offset_col = win_exp_col(&line, win->offset.cols);
	/* Do nothing if line hidden behind offset or empty. */
	if (line.render_len <= exp_offset_col)
		return 0;

	/* Calculate length to draw using expanded length and draw. */
	len_to_draw = MIN(win->size.ws_col, line.render_len - exp_offset_col);
	ret = vec_append(buf, &line.render[exp_offset_col], len_to_draw);
	return ret;
}

int
win_draw_lines(const struct win *const win, struct vec *const buf)
{
	int ret;
	unsigned short row;

	/* Set colors. */
	ret = esc_color_fg(buf, cfg_color_lines_fg);
	if (-1 == ret)
		return -1;

	for (row = 0; row + STAT_ROWS_CNT < win->size.ws_row; row++) {
		/* Draw line. */
		ret = win_draw_line(win, buf, row);
		if (-1 == ret)
			return -1;

		/* Move to the beginning of the next row. */
		ret = vec_append(buf, "\r\n", 2);
		if (-1 == ret)
			return -1;
	}

	/* End colored output. */
	ret = esc_color_end(buf);
	return ret;
}

static size_t
win_exp_col(const struct pub_line *const line, const size_t col)
{
	size_t i;
	size_t exp = 0;
	const size_t end = MIN(col, line->len);

	/* Iterate over every character in the selected area. */
	for (i = 0; i < end; i++)
		exp += str_exp(line->chars[i], exp);
	return exp;
}

char
win_file_is_dirty(const struct win *const win)
{
	return file_is_dirty(win->file);
}

const char*
win_file_path(const struct win *const win)
{
	return file_path(win->file);
}

int
win_ins_char(struct win *const win, const char ch)
{
	int ret;

	/* Insert character. */
	ret = file_ins_char(
		win->file,
		win_curr_line_idx(win),
		win_curr_line_char_idx(win),
		ch
	);
	if (-1 == ret)
		return -1;

	/* Move right after insertion. */
	ret = win_mv_right(win, 1);
	if (-1 == ret)
		return -1;

	/* Fix expanded cursor column. */
	ret = win_scroll(win);
	return ret;
}

int
win_ins_empty_line_below(struct win *const win, const size_t times)
{
	int ret;
	size_t times_i = times;

	if (0 == times)
		return 0;

	/* Remove column offsets. */
	win_mv_to_begin_of_line(win);

	/* Insert empty lines. */
	while (times_i-- > 0) {
		ret = file_ins_empty_line(win->file, win_curr_line_idx(win) + 1);
		if (-1 == ret)
			return -1;
	}

	/* Move to last inserted line. */
	ret = win_mv_down(win, times);
	return ret;
}

int
win_ins_empty_line_on_top(struct win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	/* Reove column offsets. */
	win_mv_to_begin_of_line(win);

	/* Insert empty lines. */
	while (times-- > 0) {
		ret = file_ins_empty_line(win->file, win_curr_line_idx(win));
		if (-1 == ret)
			return -1;
	}
	return 0;
}

int
win_mv_down(struct win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	while (times-- > 0) {
		/* Return if there is no more space to move down. */
		if (win->offset.rows + win->cur.row + 1 >= file_lines_cnt(win->file))
			return 0;

		/* Check that there is no space in current window. */
		if (win->cur.row + STAT_ROWS_CNT + 1 == win->size.ws_row)
			win->offset.rows++;
		else
			win->cur.row++;
	}

	/* Clamp cursor to line after move down several times. */
	ret = win_scroll(win);
	return ret;
}

int
win_mv_left(struct win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	while (times-- > 0) {
		if (win->offset.cols + win->cur.col == 0) {
			/* Check there is no next line. */
			if (win->offset.rows + win->cur.row == 0)
				break;

			/* Move to the beginning of next line if no space to move left. */
			ret = win_mv_up(win, 1);
			if (-1 == ret)
				return -1;
			win_mv_to_end_of_line(win);
		} else if (win->cur.col == 0) {
			/* We are at the right of window, but have non-zero offset. */
			win->offset.cols--;
		} else {
			/* We are have enough space to move right in the current window. */
			win->cur.col--;
		}
	}

	/* Fix expanded cursor column during left movement. */
	ret = win_scroll(win);
	return ret;
}

int
win_mv_right(struct win *const win, size_t times)
{
	int ret;
	struct pub_line line;

	if (0 == times)
		return 0;

	/* Get line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	while (times-- > 0) {
		if (win_curr_line_char_idx(win) >= line.len) {
			/* Check there is no next line. */
			if (win_curr_line_idx(win) + 1 == file_lines_cnt(win->file))
				break;

			/* Move to the beginning of next line because of end of line. */
			win_mv_to_begin_of_line(win);
			ret = win_mv_down(win, 1);
			if (-1 == ret)
				return -1;

			/* Update line data because of moving to new line. */
			ret = file_line(win->file, win_curr_line_idx(win), &line);
			if (-1 == ret)
				return -1;
		} else if (win->cur.col + 1 >= win->size.ws_col) {
			/* We are at the right of window but can shift offset. */
			win->offset.cols++;
		} else {
			/* We are have enough space to move right in the current window. */
			win->cur.col++;
		}
	}

	/* Fix expanded cursor column during right movement. */
	ret = win_scroll(win);
	return ret;
}

void
win_mv_to_begin_of_file(struct win *const win)
{
	/* Move to begin of first line. */
	win_mv_to_begin_of_line(win);
	win->offset.rows = 0;
	win->cur.row = 0;
}

void
win_mv_to_begin_of_line(struct win *const win)
{
	/* Zeroize column positions. */
	win->offset.cols = 0;
	win->cur.col = 0;
}

void
win_mv_to_end_of_file(struct win *const win)
{
	size_t lines_cnt;

	/* Get lines count. */
	lines_cnt = file_lines_cnt(win->file);

	/* Move to begin of last line. */
	win_mv_to_begin_of_line(win);

	/* Check that line on initial frame. */
	if (lines_cnt < win->size.ws_row) {
		win->offset.rows = 0;
		win->cur.row = lines_cnt - 1;
	} else {
		win->offset.rows = lines_cnt - (win->size.ws_row - STAT_ROWS_CNT);
		win->cur.row = win->size.ws_row - STAT_ROWS_CNT - 1;
	}
}

int
win_mv_to_end_of_line(struct win *const win)
{
	int ret;
	struct pub_line line;

	/* Get line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	/* Check that end of line in the current window. */
	if (line.render_len < win->offset.cols + win->size.ws_col) {
		win->cur.col = line.render_len - win->offset.cols;
	} else {
		win->offset.cols = line.render_len - win->size.ws_col + 1;
		win->cur.col = win->size.ws_col - 1;
	}

	/* Scroll after moving right. */
	ret = win_scroll(win);
	return ret;
}

int
win_mv_to_next_word(struct win *const win, size_t times)
{
	int ret;
	size_t char_idx;
	size_t word_idx;
	struct pub_line line;

	/* Get line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	while (times-- > 0) {
		/* Find next word from current position until end of line. */
		char_idx = win_curr_line_char_idx(win);
		word_idx = word_next(&line.chars[char_idx], line.len - char_idx);

		/* Check that word in the current window. */
		if (win->cur.col + word_idx < win->size.ws_col) {
			win->cur.col += word_idx;
		} else {
			win->offset.cols = char_idx + word_idx - win->size.ws_col + 1;
			win->cur.col = win->size.ws_col - 1;
		}

		/* Check that we at end of line. */
		if (char_idx + word_idx == line.len)
			break;
	}

	/* Fix expanded cursor column during right movement. */
	ret = win_scroll(win);
	return ret;
}

int
win_mv_to_prev_word(struct win *const win, size_t times)
{
	int ret;
	size_t word_i;
	struct pub_line line;

	if (0 == times)
		return 0;

	/* Get line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	while (times-- > 0) {
		/* Find next word from current position until start of line. */
		word_i = word_rnext(line.chars, win_curr_line_char_idx(win));

		/* Check that word in the current window. */
		if (word_i >= win->offset.cols) {
			win->cur.col = word_i - win->offset.cols;
		} else {
			win->offset.cols = word_i - 1;
			win->cur.col = 1;
		}

		/* Check that we at start of line. */
		if (word_i == 0)
			break;
	}

	/* Fix expanded cursor column during left movement. */
	ret = win_scroll(win);
	return ret;
}

int
win_mv_up(struct win *const win, size_t times)
{
	int ret;

	if (0 == times)
		return 0;

	while (times-- > 0) {
		/* Break if there is no more space to move up. */
		if (win->offset.rows + win->cur.row == 0)
			break;

		/* Check that there is no space in current window. */
		if (0 == win->cur.row)
			win->offset.rows--;
		else
			win->cur.row--;
	}

	/* Clamp cursor to line after move down several times. */
	ret = win_scroll(win);
	return ret;
}

struct win*
win_open(const char *const path, const int ifd, const int ofd)
{
	int ret;
	struct win *win;

	/* Allocate opaque struct. */
	win = malloc(sizeof(*win));
	if (NULL == win)
		return NULL;

	/* Open file. */
	win->file = file_open(path);
	if (NULL == win->file)
		goto err_free_opaque;

	/* Initialize offset and cursor. */
	memset(&win->offset, 0, sizeof(win->offset));
	memset(&win->cur, 0, sizeof(win->cur));

	/* Initialize terminal with accepted descriptors. */
	ret = term_init(ifd, ofd);
	if (-1 == ret)
		goto err_free_opaque_and_close;

	/* Get window size. */
	ret = term_get_win_size(&win->size);
	if (-1 == ret)
		goto err_clean_all;
	return win;
err_clean_all:
	/* Errors checking here is useless. */
	term_deinit();
err_free_opaque_and_close:
	file_close(win->file);
err_free_opaque:
	free(win);
	return NULL;
}

size_t
win_save_file(struct win *const win)
{
	size_t len;

	len = file_save(win->file, NULL);
	return len;
}

size_t
win_save_file_to_spare_dir(struct win *const win, char *const path, size_t len)
{
	size_t bytes;

	bytes = file_save_to_spare_dir(win->file, path, len);
	return bytes;
}

static int
win_scroll(struct win *const win)
{
	int ret;

	win_scroll_overflowed_cur(win);

	ret = win_scroll_to_line(win);
	if (-1 == ret)
		return -1;

	ret = win_scroll_exp_col(win);
	return ret;
}

static int
win_scroll_exp_col(struct win *const win)
{
	int ret;
	struct pub_line line;
	size_t exp_offset_col;
	size_t exp_col;

	/* Get current line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	while (1) {
		/* Get expanded hidden column and expanded viewed column. */
		exp_col = win_exp_col(&line, win->offset.cols + win->cur.col);
		exp_offset_col = win_exp_col(&line, win->offset.cols);

		/* Check that diff between expansion is not greater than window's width. */
		if (exp_col - exp_offset_col < win->size.ws_col)
			break;

		/* Shift to view pointed content. */
		win->offset.cols++;
		win->cur.col--;
	}
	return 0;
}

static void
win_scroll_overflowed_cur(struct win *const win)
{
	if (win->cur.row + STAT_ROWS_CNT >= win->size.ws_row) {
		win->offset.rows += win->cur.row + STAT_ROWS_CNT + 1 - win->size.ws_row;
		win->cur.row = win->size.ws_row - STAT_ROWS_CNT - 1;
	}
	if (win->cur.col >= win->size.ws_col) {
		win->offset.cols += win->cur.col + 1 - win->size.ws_col;
		win->cur.col = win->size.ws_col - 1;
	}
}

static int
win_scroll_to_line(struct win *const win)
{
	int ret;
	struct pub_line line;

	/* Get current line. */
	ret = file_line(win->file, win_curr_line_idx(win), &line);
	if (-1 == ret)
		return -1;

	/* Check that cursor out of the line. */
	if (win->offset.cols + win->cur.col > line.len) {
		/* Check that line not in the window. */
		if (line.len <= win->offset.cols) {
			win->offset.cols = 0 == line.len ? 0 : line.len - 1;
			win->cur.col = 0 == line.len ? 0 : 1;
		} else {
			win->cur.col = line.len - win->offset.cols;
		}
	}
	return 0;
}

int
win_search_bwd(struct win *const win, const char *const query)
{
	int ret;
	size_t idx;
	size_t pos;

	/* Prepare indexes. */
	idx = win_curr_line_idx(win);
	pos = win_curr_line_char_idx(win);

	/* Search with accepted query. */
	ret = file_search_bwd(win->file, &idx, &pos, query);
	if (-1 == ret)
		return -1;
	if (0 == ret)
		return 0;

	/* Move to begin of line to easily move right to result later. */
	win_mv_to_begin_of_line(win);

	/* Move to result's line. */
	ret = win_mv_up(win, win_curr_line_idx(win) - idx);
	if (-1 == ret)
		return -1;

	/* Move to result on current line. */
	ret = win_mv_right(win, pos);
	return ret;
}

int
win_search_fwd(struct win *const win, const char *const query)
{
	int ret;
	size_t idx;
	size_t pos;

	/* Move forward to not collide with previous result. */
	ret = win_mv_right(win, 1);
	if (-1 == ret)
		return -1;

	/* Prepare indexes. */
	idx = win_curr_line_idx(win);
	pos = win_curr_line_char_idx(win);

	/* Search with accepted query. */
	ret = file_search_fwd(win->file, &idx, &pos, query);
	if (-1 == ret)
		return -1;
	if (0 == ret) {
		/* Move back to start position if no results during forward searching. */
		ret = win_mv_left(win, 1);
		return ret;
	}

	if (win_curr_line_idx(win) != idx)
		/* Move to begin of line to easily move right to result on the next line. */
		win_mv_to_begin_of_line(win);

	/* Move to result's line. */
	ret = win_mv_down(win, idx - win_curr_line_idx(win));
	if (-1 == ret)
		return -1;

	/* Move to result on current line. */
	ret = win_mv_right(win, pos);
	return ret;
}

struct winsize
win_size(const struct win *const win)
{
	return win->size;
}

int
win_upd_size(struct win *const win)
{
	int ret;

	/* Update size using terminal. */
	ret = term_get_win_size(&win->size);
	if (-1 == ret)
		return -1;

	/* Scroll after resize. */
	ret = win_scroll(win);
	return ret;
}
