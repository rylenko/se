#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "cur.h"
#include "term.h"
#include "win.h"

/* Window size change handler */
static void win_handle_sigwinch(int _num);

/* Registers signal handlers for the window. */
static void win_reg_sig_handlers(Win *const win);

/* Unregisters signal handlers for the window. */
static void win_unreg_sig_handlers(void);

/* Global pointer to use in signal handlers */
Win *sig_win = NULL;

void
win_deinit(void)
{
	/* Unregister signal handlers */
	win_unreg_sig_handlers();

	/* Deinitialize terminal */
	term_deinit();
}

static void
win_handle_sigwinch(int _num)
{
	(void)_num;
	/* Update window size using terminal */
	term_get_win_size(&sig_win->size);
}

void
win_init(Win *const win, const int ifd, const int ofd)
{
	/* Initialize cursor and offset */
	cur_init(&win->cur);
	win->offset.cols = 0;
	win->offset.rows = 0;

	/* Initialize terminal with accepted descriptors */
	term_init(ifd, ofd);
	/* Get window size */
	term_get_win_size(&win->size);

	/* Register signal handlers */
	win_reg_sig_handlers(win);
}

static void
win_reg_sig_handlers(Win *const win)
{
	/* Set pointer for signal handlers */
	sig_win = win;

	/* Register window size change handler */
	if (signal(SIGWINCH, win_handle_sigwinch) == SIG_ERR)
		err(EXIT_FAILURE, "Failed to set window size change handler");
}

static void
win_unreg_sig_handlers(void)
{
	/* Unset global pointer to window */
	sig_win = NULL;

	/* Unregister window size change handler */
	if (signal(SIGWINCH, SIG_DFL) == SIG_ERR)
		err(EXIT_FAILURE, "Failed to unset window size change handler");
}
