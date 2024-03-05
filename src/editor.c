#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "editor.h"
#include "term.h"

/* Structure with editor parameters. */
static struct {
	char need_to_quit;
	const char *path;
	struct winsize win_size;
} editor;

/* Clears the screen. Then sets the cursor to the beginning of the screen. */
static void editor_clear_scr(void);

/* Updates the size and checks that everything fits on the screen. */
static void editor_handle_sig_win_ch(int num);

/*
Requests the size from the terminal and sets it in the appropriate field.

To update the window size after it has been changed, use the handler
`editor_handle_sig_win_ch`.
*/
static void editor_update_win_size(void);

static void
editor_clear_scr(void)
{
	unsigned short col_i;
	unsigned short row_i;
	Buf buf = buf_alloc();
	/* Go home before clearing */
	term_go_home(&buf);

	/* Clear the screen */
	for (row_i = 0; row_i < editor.win_size.ws_row; row_i++)
		for (col_i = 0; col_i < editor.win_size.ws_col; col_i++)
			/* TODO: Avoid many reallocations in `buf_write` */
			buf_write(&buf, " ", 1);

	/* Go home after clearing */
	term_go_home(&buf);
	/* Flush and free the buffer */
	term_flush(&buf);
	buf_free(buf);
}

static void
editor_handle_sig_win_ch(int num)
{
	(void)num;
	editor_update_win_size();
	/* TODO: check cursor position fits on the screen */
	editor_refresh_scr();
}

char
editor_need_to_quit(void)
{
	return editor.need_to_quit;
}

void
editor_open(const char *path)
{
	assert(path);

	editor.need_to_quit = 0;
	editor.path = path;

	/* Update window size and register the handler of window size changing */
	editor_update_win_size();
	signal(SIGWINCH, editor_handle_sig_win_ch);
}

void
editor_refresh_scr(void)
{
	Buf buf = buf_alloc();

	/* Assert that we do not need to quit */
	assert(!editor.need_to_quit);
	/* Hide cursor */
	term_hide_cur(&buf);

	/* Print lines */
	editor_clear_scr();
	buf_writef(&buf, "Rows: %d\r\n", editor.win_size.ws_row);
	buf_writef(&buf, "Path: \"%s\"\r\n", editor.path);

	/* Show cursor */
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

void
editor_wait_and_proc_key_press(void)
{
	char key = term_wait_key_press();

	/* Assert that we do not need to quit */
	assert(!editor.need_to_quit);

	/* Quit key */
	if (key == 'q') {
		editor.need_to_quit = 1;
		editor_clear_scr();
	}
}
