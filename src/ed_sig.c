#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include "ed_draw.h"
#include "ed.h"
#include "ed_sig.h"
#include "term.h"

/* Global pointer to use in handlers */
Ed *ed = NULL;

/* Handles window size changing signal. */
static void ed_sig_handle_win_size_ch(int _num);

void
ed_sig_reg(Ed *ed_local)
{
	ed = ed_local;
	/* Window size change */
	if (signal(SIGWINCH, ed_sig_handle_win_size_ch) == SIG_ERR) {
		err(EXIT_FAILURE, "Failed to set window size change handler");
	}
}

static void
ed_sig_handle_win_size_ch(int _num)
{
	(void)_num;
	term_get_win_size(&ed->win_size);
	ed_fix_cur(ed);
	ed_draw(ed);
}
