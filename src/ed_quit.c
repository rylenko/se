/* Ed, ed_msg_set */
#include "ed.h"
#include "ed_quit.h"
/* rows_free */
#include "row.h"

static const char *const ed_quit_presses_rem_msg_fmt = \
	"There are unsaved changes. Presses remain to quit: %hhu.";

char
ed_quit_done(const Ed *const ed)
{
	return ed->quit_presses_rem == 0;
}

void
ed_quit_try(Ed *const ed)
{
	ed->quit_presses_rem--;
	if (ed_quit_done(ed)) {
		free(ed->path);
		rows_free(&ed->rows);
	} else {
		ed_msg_set(ed, ed_quit_presses_rem_msg_fmt, ed->quit_presses_rem);
	}
}
