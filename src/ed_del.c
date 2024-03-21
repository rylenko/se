#include "ed.h"
#include "ed_del.h"
#include "ed_mv.h"
#include "math.h"
#include "row.h"
#include "rows.h"

static const char *const ed_del_only_one_row_msg = \
	"It is forbidden to delete a single row.";

void
ed_del(Ed *const ed)
{
	const size_t f_row_i = ed->offset_row + ed->cur.y;
	const size_t f_col_i = ed->offset_col + ed->cur.x;
	Row *const row = &ed->rows.arr[f_row_i];
	if (f_col_i > 0) {
		/* Delete character and update render */
		row_del(row, f_col_i - 1);
		row_upd_render(row);
		ed_mv_left(ed, 1);
	} else if (f_row_i > 0) {
		/* Move left first to have cursor at end of previous row */
		ed_mv_left(ed, 1);
		/* Extends previous row with current and delete current row */
		rows_extend_with_next(&ed->rows, f_row_i - 1);
		row_upd_render(&ed->rows.arr[f_row_i - 1]);
	}
	ed_on_f_ch(ed);
}

void
ed_del_row(Ed *const ed, size_t times)
{
	if (1 == ed->rows.cnt) {
		ed_set_msg(ed, ed_del_only_one_row_msg);
	} else if (times > 0) {
		/* Get real repeat times */
		times = MIN(ed->rows.cnt - ed->offset_row - ed->cur.y, times);
		/* The file must contain at least one row */
		if (times == ed->rows.cnt) {
			times--;
		}
		/* Remove x offsets and delete the row */
		ed_mv_begin_of_row(ed);
		while (times-- > 0) {
			rows_del(&ed->rows, ed->offset_row + ed->cur.y);
		}
		ed_on_f_ch(ed);
	}
}
