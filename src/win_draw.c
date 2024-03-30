#include "buf.h"
#include "esc.h"
#include "line.h"
#include "math.h"
#include "win.h"

void
win_draw_cur(const Win *const win, Buf *const buf)
{
	unsigned short col = 0;
	size_t idx;
	unsigned short row = 0;

	/* Offset from previous rows */
	for (idx = win->top_line_idx; idx < win->curr_line_idx; idx++)
		/* Determining how many rows a line takes up */
		row += win->file.lines.arr[idx].render_len / win->size.ws_col + 1;

	/* Offset form current row */
	row += win->curr_line_render_idx / win->size.ws_col;
	col += win->curr_line_render_idx % win->size.ws_col;

	/* Set the cursor */
	esc_cur_set(buf, row, col);
}

void
win_draw_lines(Win *const win, Buf *const buf)
{
	size_t len_to_draw;
	size_t line_idx;
	size_t row;
	const Line *line;

	/* Draw rows */
	for (
		row = 0, line_idx = win->top_line_idx;
		row + WIN_STAT_ROWS_CNT < win->size.ws_row;
		row++, line_idx++
	) {
		/* Check no more lines */
		if (line_idx >= win->file.lines.cnt) {
			buf_write(buf, "~", 1);
		} else {
			/* Get current line by its index */
			line = &win->file.lines.arr[line_idx];

			if (line->render_len > 0) {
				/* We determine how much length it is possible to draw and draw */
				len_to_draw = MIN(
					line->render_len,
					(win->size.ws_row - row - WIN_STAT_ROWS_CNT) * win->size.ws_col
				);
				buf_write(buf, line->render, len_to_draw);

				/* Determine how many full rows spent on this line */
				row += len_to_draw / win->size.ws_col;

				/* Reserve one more line if the next character of the line goes to it */
				if (len_to_draw % win->size.ws_col == 0)
					buf_write(buf, "\r\n", 2);

				/* Update bottom row's index */
				win->bot_line_idx = line_idx;
			}
		}

		/* Move to start of next row */
		buf_write(buf, "\r\n", 2);
	}
}
