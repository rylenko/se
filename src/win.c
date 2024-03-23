#include <signal.h>
#include <string.h>
#include "pos.h"
#include "term.h"
#include "win.h"

/* Window size change handler */
static void win_handle_sigwinch(int _num);

/* Registers signal handlers for the window. */
static void win_reg_sig_handlers(Win *const win);

/* Global pointer to use in signal handlers */
Win *win_global = NULL;

void
win_deinit(Win *const win)
{
	/* Deinitialize terminal */
	term_deinit();
}

static void
win_handle_sigwinch(int _num)
{
	(void)_num;
	/* Update window size using terminal */
	term_get_win_size(&win_global->size);
}

void
win_init(Win *const win, const int ifd, const int ofd)
{
	/* Initialize position */
	pos_init(&win->pos);

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
	win_global = win;
	/* Register window size change handler */
	if (signal(SIGWINCH, win_handle_sigwinch) == SIG_ERR)
		err(EXIT_FAILURE, "Failed to set winow size change handler");
}
