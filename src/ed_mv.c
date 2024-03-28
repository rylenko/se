#include "ed.h"
#include "ed_mv.h"

void
ed_mv_to_begin_of_file(Ed *const ed)
{
	/* Zeroize offsets and positions */
	ed->file.pos.row = 0;
	ed->win.offset.rows = 0;
	ed->win.cur.row = 0;
	ed_mv_to_begin_of_row(ed);
}

void
ed_mv_to_begin_of_row(Ed *const ed)
{
	/* Zeroize column offset and positions */
	ed->file.pos.col = 0;
	ed->win.offset.cols = 0;
	ed->win.cur.col = 0;
}

void
ed_mv_to_end_of_file(Ed *const ed)
{
	/* Zeroize column offset and positions */
	ed_mv_to_begin_of_row(ed);

	/* Check that row on initial window */
	if (ed->file.rows.cnt < ed->win.size.ws_row) {
		ed->win.offset.rows = 0;
		ed->win.cur.row = ed->file.rows.cnt - 1;
	} else {
		/* Shift so that only the last window is shown in full */
		ed->win.offset.rows = ed->file.rows.cnt + 1 - ed->win.size.ws_row;
		/* Move cursor to end of window. Do not forget about status row */
		ed->win.cur.row = ed->win.size.ws_row - 2;

	}

	/* Update file position using window's offset and cursor */
	ed->file.pos.row = ed->win.offset.rows + ed->win.cur.row;
}
