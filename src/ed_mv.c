#include "cfg.h"
#include "ed.h"
#include "ed_mv.h"
#include "math.h"
#include "word.h"

void
ed_mv_begin_of_f(Ed *const ed)
{
	ed_mv_begin_of_row(ed);
	ed->offset_row = 0;
	ed->cur.y = 0;
}

void
ed_mv_begin_of_row(Ed *const ed)
{
	ed->offset_col = 0;
	ed->cur.x = 0;
}

void
ed_mv_down(Ed *const ed, size_t times)
{
	size_t used_times;
	if (ed->offset_row + ed->cur.y + 1 < ed->rows.cnt) {
		/* Move cursor down in current window */
		used_times = MIN((size_t)(ed->win_size.ws_row - 2 - ed->cur.y), times);
		ed->cur.y += used_times;
		times -= used_times;
		/* Move down by offset shifting */
		ed->offset_row += MIN(
			ed->rows.cnt - ed->offset_row - ed->cur.y + 1,
			times
		);
		/* Fix cursor to fit on the row */
		ed_fix_cur(ed);
	}
}

void
ed_mv_end_of_f(Ed *const ed)
{
	ed_mv_begin_of_row(ed);
	/* Check that ro on initial window */
	if (ed->rows.cnt < ed->win_size.ws_row) {
		ed->offset_row = 0;
		ed->cur.y = ed->rows.cnt - 1;
	} else {
		ed->offset_row = ed->rows.cnt + 1 - ed->win_size.ws_row;
		ed->cur.y = ed->win_size.ws_row - 2;
	}
}

void
ed_mv_end_of_row(Ed *const ed)
{
	const Row *const row = &ed->rows.arr[ed->offset_row + ed->cur.y];
	if (row->len < ed->offset_col + ed->win_size.ws_col) {
		/* End of row on the window */
		ed->cur.x = row->len - ed->offset_col;
	} else {
		/* Offset to see end of row on the window */
		ed->offset_col = row->len - ed->win_size.ws_col + 1;
		ed->cur.x = ed->win_size.ws_col - 1;
	}
}

void
ed_mv_left(Ed *const ed, size_t times)
{
	for (; times > 0; times--) {
		/* Move to previous row if exists and cursor at start of row */
		if (ed->offset_col + ed->cur.x == 0) {
			if (ed->offset_row + ed->cur.y == 0) {
				break;
			}
			ed_mv_up(ed, 1);
			ed_mv_end_of_row(ed);
		} else {
			if (ed->cur.x == 0) {
				/* We are at the left of window */
				ed->offset_col--;
			} else {
				/* We are have enough space to move left on the window */
				ed->cur.x--;
			}
		}
	}
}

void
ed_mv_next_word(Ed *const ed, size_t times)
{
	/* Find next word */
	const Row *const row = &ed->rows.arr[ed->offset_row + ed->cur.y];
	size_t f_col_i;
	size_t word_i;
	while (times-- > 0) {
		/* Find next word */
		f_col_i = ed->offset_col + ed->cur.x;
		word_i = word_next(row->cont + f_col_i, row->len - f_col_i);
		/* Check word on the window */
		if (word_i + ed->cur.x < ed->win_size.ws_col) {
			ed->cur.x += word_i;
		} else {
			ed->offset_col = f_col_i + word_i - ed->win_size.ws_col + 1;
			ed->cur.x = ed->win_size.ws_col - 1;
		}
		/* End of row */
		if (word_i + f_col_i == row->len) {
			break;
		}
	}
}

void
ed_mv_prev_word(Ed *const ed, size_t times)
{
	size_t f_col_i;
	size_t word_i;
	const Row *const row = &ed->rows.arr[ed->offset_row + ed->cur.y];

	while (times-- > 0) {
		/* Find previous word */
		f_col_i = ed->offset_col + ed->cur.x;
		word_i = word_rnext(row->cont, f_col_i);
		/* Check word on the window */
		if (word_i >= ed->offset_col) {
			ed->cur.x = word_i - ed->offset_col;
		} else {
			ed->offset_col = word_i;
			ed->cur.x = 1;
		}
		/* Start of row */
		if (word_i == 0) {
			break;
		}
	}
}

void
ed_mv_right(Ed *const ed, size_t times)
{
	const Row *row = &ed->rows.arr[ed->offset_row + ed->cur.y];
	for (; times > 0; times--) {
		/* Move to next row if exists and cursor at start of row */
		if (ed->offset_col + ed->cur.x == row->len) {
			if (ed->offset_row + ed->cur.y + 1 == ed->rows.cnt) {
				break;
			}
			ed_mv_down(ed, 1);
			ed_mv_begin_of_row(ed);
			row++;
		} else {
			if (ed->win_size.ws_col - 1 == ed->cur.x) {
				/* We are at the right of window */
				ed->offset_col++;
			} else {
				/* We are have enough space to move right on the window */
				ed->cur.x++;
			}
		}
	}
}

void
ed_mv_up(Ed *const ed, size_t times)
{
	size_t used_times;
	if (ed->offset_row + ed->cur.y > 0) {
		/* Move cursor up in current window */
		used_times = MIN(ed->cur.y, times);
		ed->cur.y -= used_times;
		times -= used_times;
		/* Move up by offset shifting */
		ed->offset_row -= MIN(ed->offset_row, times);
		/* Fix cursor to fit on the row */
		ed_fix_cur(ed);
	}
}
