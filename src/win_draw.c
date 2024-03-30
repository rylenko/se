#include "buf.h"
#include "cfg.h"
#include "esc.h"
#include "line.h"
#include "math.h"
#include "win.h"

void
win_draw_cur(const Win *const win, Buf *const buf)
{
	/* Expand offset and file columns */
	size_t exp_offset_col = win_exp_col(win, win->offset.cols);
	size_t exp_file_col = win_exp_col(win, win->offset.cols + win->cur.col);

	/* Substract expanded columns to get real column in the window */
	esc_cur_set(buf, win->cur.row, exp_file_col - exp_offset_col);
}

void
win_draw_lines(const Win *const win, Buf *const buf)
{
	size_t exp_offset_col;
	size_t row;
	size_t len_to_draw;
	const Line *line;

	for (row = 0; row + STAT_ROWS_CNT < win->size.ws_row; row++) {
		/* Checking if there is a line to draw at this row */
		if (win->offset.rows + row >= win->file.lines.cnt) {
			buf_write(buf, "~", 1);
		} else {
			/* Get current line */
			line = &win->file.lines.arr[win->offset.rows + row];
			/* Draw line if not empty and not hidden behind offset */
			if (line->render_len > win->offset.cols) {
				/* Expand offset column with tabs to get render's length to draw */
				exp_offset_col = win_exp_col(win, win->offset.cols);
				len_to_draw = MIN(win->size.ws_col, line->render_len - exp_offset_col);

				buf_write(buf, &line->render[exp_offset_col], len_to_draw);
			}
		}

		/* Move to the beginning of the next row */
		buf_write(buf, "\r\n", 2);
	}
}
