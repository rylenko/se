#include <assert.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "editor.h"
#include "term.h"

static struct {
	const char *path;
	struct winsize win_size;
} editor;


/* Updates the size and checks that everything fits on the screen. */
static void editor_handle_sig_win_ch(int num);

/*
Requests the size from the terminal and sets it in the appropriate field.

To update the window size after it has been changed, use the handler
`editor_handle_sig_win_ch`.
*/
static void editor_update_win_size(void);

static void
editor_handle_sig_win_ch(int num)
{
	(void)num;
	editor_update_win_size();
	/* TODO: check cursor position fits on the screen */
	editor_refresh_scr();
}

void
editor_init(void)
{
	editor_update_win_size();
	signal(SIGWINCH, editor_handle_sig_win_ch);
}

void
editor_open(const char *path)
{
	assert(path);
	editor.path = path;
}

void
editor_refresh_scr(void)
{
	unsigned short row_i;
	Buf buf = buf_alloc();

	/* Prepare for file content */
	term_hide_cur(&buf);
	term_go_home(&buf);

	/* Print lines */
	for (row_i = 0; row_i < editor.win_size.ws_row; row_i++)
		buf_write(&buf, "~\r\n", 3);

	/* Recovery after file content */
	term_show_cur(&buf);

	/* Flush and free the buffer */
	term_flush(&buf);
	buf_free(buf);
}

static void
editor_update_win_size(void)
{
	term_get_win_size(&editor.win_size);
}
