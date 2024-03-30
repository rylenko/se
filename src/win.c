#include <signal.h>
#include "file.h"
#include "term.h"
#include "win.h"

/* Updates window's size using terminal. */
static void win_upd_size(Win *const win);

void
win_close(Win *const win)
{
	term_deinit();
	file_close(&win->file);
}

void
win_handle_signal(Win *const win, const int signal)
{
	if (SIGWINCH == signal)
		win_upd_size(win);
}

void
win_open(Win *const win, const char *const path, const int ifd, const int ofd)
{
	/* Open file */
	file_open(&win->file, path);
	/* Set lines */
	win->top_line_idx = 0;
	win->curr_line_idx = 0;
	win->curr_line_cont_idx = 0;
	win->curr_line_render_idx = 0;
	win->bot_line_idx = 0;

	/* Initialize terminal with accepted descriptors */
	term_init(ifd, ofd);
	/* Get window size */
	term_get_win_size(&win->size);
}

static void
win_upd_size(Win *const win)
{
	/* Update size using terminal */
	term_get_win_size(&win->size);

	/* TODO: - */
}
