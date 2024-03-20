#include "cfg.h"
#include "ed.h"
#include "ed_ins.h"
#include "ed_mv.h"
#include "row.h"
#include "rows.h"
#include "math.h"
#include "mode.h"

void
ed_ins(Ed *const ed, const char ch)
{
	row_ins(
		&ed->rows.arr[ed->offset_row + ed->cur.y],
		ed->offset_col + ed->cur.x,
		ch
	);
	ed_mv_right(ed, 1);
	ed_on_f_ch(ed);
}

void
ed_ins_break(Ed *const ed)
{
	rows_break(&ed->rows, ed->offset_row + ed->cur.y, ed->offset_col + ed->cur.x);
	ed_mv_begin_of_row(ed);
	ed_mv_down(ed, 1);
}

void
ed_ins_row_below(Ed *const ed, size_t times)
{
	size_t ins_i = ed->offset_row + ed->cur.y + 1;
	times = MIN(times, CFG_INS_ROW_LIMIT);
	ed_mv_begin_of_row(ed);
	ed_mv_down(ed, times);
	while (times-- > 0) {
		rows_ins(&ed->rows, ins_i++, row_empty());
	}
	ed->mode = MODE_INS;
	ed_on_f_ch(ed);
}

void
ed_ins_row_top(Ed *const ed, size_t times)
{
	ed_mv_begin_of_row(ed);
	while (times-- > 0) {
		rows_ins(&ed->rows, ed->offset_row + ed->cur.y, row_empty());
	}
	ed->mode = MODE_INS;
	ed_on_f_ch(ed);
}
