/* Ed */
#include "ed.h"
#include "ed_cur.h"
/* Row */
#include "row.h"
/* MIN */
#include "math.h"

void
ed_cur_fix(Ed *const ed)
{
	const Row *const row = &ed->rows.arr[ed->offset_row + ed->cur.y];
	const size_t f_col_i = ed->offset_col + ed->cur.x;
	size_t col_diff;

	/* Clamp cursor on the window */
	ed->cur.y = MIN(ed->cur.y, ed->win_size.ws_row - 2);
	ed->cur.x = MIN(ed->cur.x, ed->win_size.ws_col - 1);

	/* Fix x coordinate if current row does not has enough length */
	if (f_col_i > row->len) {
		col_diff = f_col_i - row->len;

		if (ed->cur.x < col_diff) {
			/* Return row on the window */
			ed->offset_col -= col_diff - ed->cur.x;
			ed->cur.x = 0;
			/* Show last character of the row if exists */
			if (row->len > 0) {
				ed->offset_col--;
			}
		} else {
			/* Offset the cursor */
			ed->cur.x -= col_diff;
		}
	}
}
