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

static void editor_update_win_size(void);

void
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
	Buf buf = buf_alloc();

	/* Prepare for file content */
	term_hide_cur(&buf);
	term_go_home(&buf);

	buf_write(&buf, "Hello,", 6);
	buf_write(&buf, " world!\r\n", 9);
	buf_writef(&buf, "Filename: %s\r\n", editor.path);
	buf_writef(&buf, "Rows: %hu\r\n", editor.win_size.ws_row);
	buf_writef(&buf, "Cols: %hu\r\n", editor.win_size.ws_col);

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
