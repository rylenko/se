#include <signal.h>
#include <string.h>
#include "cfg.h"
#include "file.h"
#include "line.h"
#include "math.h"
#include "term.h"
#include "win.h"

/* Updates window's size using terminal. */
static void win_upd_size(Win *const win);

void
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
	term_deinit();
	file_close(&win->file);
}

size_t
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

void
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

Line*
win_get_curr_line(const Win *const win)
{
	return win_get_line(win, win->offset.rows + win->cur.row);
}

Line*
win_get_line(const Win *const win, const size_t idx)
{
	return file_get(&win->file, idx);
}

void
win_handle_signal(Win *const win, const int signal)
{
	if (SIGWINCH == signal)
		win_upd_size(win);
}

void
win_open(Win *const win, const char *const path, const int ifd, const int ofd)
{
	/* Open file */
	file_open(&win->file, path);
	/* Initialize offset and cursor */
	memset(&win->offset, 0, sizeof(win->offset));
	memset(&win->cur, 0, sizeof(win->cur));

	/* Initialize terminal with accepted descriptors */
	term_init(ifd, ofd);
	/* Get window size */
	term_get_win_size(&win->size);
}

size_t
win_save_file(Win *const win)
{
	return file_save(&win->file, NULL);
}

size_t
win_save_file_to_spare_dir(Win *const win, char *const  path, size_t len)
{
	return file_save_to_spare_dir(&win->file, path, len);
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
