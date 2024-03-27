#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "cur.h"
#include "term.h"
#include "win.h"

void
win_deinit(void)
{
	/* Deinitialize terminal */
	term_deinit();
}

void
win_handle_signal(Win *const win, const int signal)
{
	if (SIGWINCH == signal)
		/* Update window size using terminal */
		term_get_win_size(&win->size);
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
}
