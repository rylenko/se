#include "ed.h"
#include "ed_mv.h"

void
ed_mv_to_begin_of_file(Ed *const ed)
{
	/* Zeroize offset and positions */
	ed->file.pos.row = 0;
	ed->win.offset.rows = 0;
	ed->win.cur.row = 0;
	ed_mv_to_begin_of_row(ed);
}

void
ed_mv_to_begin_of_row(Ed *const ed)
{
	/* Zeroize offset and positions */
	ed->file.pos.col = 0;
	ed->win.offset.cols = 0;
	ed->win.cur.col = 0;
}
