#include "math.h"
#include "win.h"
#include "win_mv.h"

void
win_mv_down(Win *const win, size_t times)
{
	while (times-- > 0) {
		/* Break if there is no more space to move down */
		if (win->offset.rows + win->cur.row + 1 >= win->file.lines.cnt)
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

void
win_mv_left(Win *const win, size_t times)
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
}

void
win_mv_right(Win *const win, size_t times)
{
	const Line *line = win_get_curr_line(win);

	while (times-- > 0) {
		/* Move to the beginning of next line if there is not space to move right */
		if (win->offset.cols + win->cur.col == line->len) {
			/* Check there is no next line */
			if (win->offset.rows + win->cur.row + 1 == win->file.lines.cnt)
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

	win_fix_exp_cur_col(win);
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
	/* Move to begin of last line */
	win_mv_to_begin_of_line(win);

	/* Check that line on initial frame */
	if (win->file.lines.cnt < win->size.ws_row) {
		win->offset.rows = 0;
		win->cur.row = win->file.lines.cnt - 1;
	} else {
		win->offset.rows = win->file.lines.cnt - (win->size.ws_row - STAT_ROWS_CNT);
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
win_mv_up(Win *const win, size_t times)
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
	win_clamp_cur_to_line(win);
}
