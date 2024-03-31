#include "ed.h"
#include "ed_mv.h"
#include "win_mv.h"

void
ed_mv_down(Ed *const ed)
{
	win_mv_down(&ed->win, ed_get_repeat_times(ed));
}

void
ed_mv_left(Ed *const ed)
{
	win_mv_left(&ed->win, ed_get_repeat_times(ed));
}

void
ed_mv_right(Ed *const ed)
{
	win_mv_right(&ed->win, ed_get_repeat_times(ed));
}

void
ed_mv_up(Ed *const ed)
{
	win_mv_up(&ed->win, ed_get_repeat_times(ed));
}

void
ed_mv_to_begin_of_file(Ed *const ed)
{
	win_mv_to_begin_of_file(&ed->win);
}

void
ed_mv_to_begin_of_line(Ed *const ed)
{
	win_mv_to_begin_of_line(&ed->win);
}

void
ed_mv_to_end_of_file(Ed *const ed)
{
	win_mv_to_end_of_file(&ed->win);
}

void
ed_mv_to_end_of_line(Ed *const ed)
{
	win_mv_to_end_of_line(&ed->win);
}

void
ed_mv_to_next_word(Ed *const ed)
{
	win_mv_to_next_word(&ed->win, ed_get_repeat_times(ed));
}

void
ed_mv_to_prev_word(Ed *const ed)
{
	win_mv_to_prev_word(&ed->win, ed_get_repeat_times(ed));
}
