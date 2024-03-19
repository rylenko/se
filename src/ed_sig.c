/* err */
#include <err.h>
/* SIGWINCH, SIG_ERR, signal */
#include <signal.h>
/* EXIT_FAILURE */
#include <stdlib.h>
/* ed_cur_fix */
#include "ed_cur.h"
/* ed_draw */
#include "ed_draw.h"
/* Ed */
#include "ed.h"
#include "ed_sig.h"
/* term_get_win_size */
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
	ed_cur_fix(ed);
	ed_draw(ed);
}
