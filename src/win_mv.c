#include "win.h"
#include "win_mv.h"

void
win_mv_to_begin_of_line(Win *const win)
{
	/* Move to begin of line's content */
	win->curr_line_cont_idx = 0;
	win->curr_line_render_idx = 0;
}

void
win_mv_to_end_of_line(Win *const win)
{
	const Line *const line = &win->file.lines.arr[win->curr_line_idx];
	/* Set content index to content length */
	win->curr_line_cont_idx = line->len;
	win->curr_line_render_idx = line->render_len;
}
