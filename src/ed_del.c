#include "cfg.h"
#include "ed.h"
#include "ed_del.h"
#include "win_del.h"

void
ed_del_line(Ed *const ed)
{
	/* Try to delete lines or set error message */
	if (win_del_line(&ed->win, ed_get_repeat_times(ed)) < 0)
		ed_set_msg(ed, "A single line in a file cannot be deleted.");
	else
		/* Set quit presses count after file change */
		ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
}
