#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "color.h"
#include "editor.h"
#include "err.h"
#include "key.h"
#include "term.h"

/* Structure with editor parameters. */
static struct {
	char need_to_quit;
	const char *path;
	struct winsize win_size;
} editor;

/* Clears the screen. Then sets the cursor to the beginning of the screen. */
static void editor_clear_scr(Buf *buf);

/* Updates the size and checks that everything fits on the screen. */
static void editor_handle_sig_win_ch(int num);

/*
Requests the size from the terminal and sets it in the appropriate field.

To update the window size after it has been changed, use the handler
`editor_handle_sig_win_ch`.
*/
static void editor_update_win_size(void);

/* Write lines in the buffer. */
static void editor_write_lines(Buf *buf);

/* Write static in the buffer. */
static void editor_write_status(Buf *buf);

static void
editor_clear_scr(Buf *buf)
{
	unsigned short col_i;
	unsigned short row_i;
	/* Go home before clearing */
	term_go_home(buf);
	/* Clear the screen */
	for (row_i = 0; row_i < editor.win_size.ws_row; row_i++)
		for (col_i = 0; col_i < editor.win_size.ws_col; col_i++)
			/* TODO: Avoid many reallocations in `buf_write` */
			buf_write(buf, " ", 1);
	/* Go home after clearing */
	term_go_home(buf);
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
	FILE *f;

	/* Initialize */
	editor.need_to_quit = 0;
	editor.path = path;

	/* Update window size and register the handler of window size changing */
	editor_update_win_size();
	signal(SIGWINCH, editor_handle_sig_win_ch);

	/* Open the file  */
	if (!(f = fopen(path, "r")))
		err("Failed to open \"%s\":", path);

	/* Close the file after read */
	fclose(f);
}

void
editor_refresh_scr(void)
{
	/* Allocate new buffer, hide cursor and clear the screen */
	Buf buf = buf_alloc();
	term_hide_cur(&buf);
	editor_clear_scr(&buf);

	/* Write content if we do not quit yet */
	if (!editor.need_to_quit) {
		editor_write_lines(&buf);
		editor_write_status(&buf);
	}

	/* Show cursor, flush and free the buffer */
	term_show_cur(&buf);
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
	/* Assert that we do not need to quit */
	assert(!editor.need_to_quit);

	switch (term_wait_key_press()) {
	/* Quit */
	case KEY_CTRL_Q:
		editor.need_to_quit = 1;
		break;
	}
}

static void
editor_write_lines(Buf *buf)
{
	unsigned short row_i;
	/* Assert that we do not need to quit */
	assert(!editor.need_to_quit);

	buf_writef(buf, "Rows: %d\r\n", editor.win_size.ws_row);
	buf_writef(buf, "Path: \"%s\"\r\n", editor.path);
	for (row_i = 2; row_i < editor.win_size.ws_row - 1; row_i++)
		buf_write(buf, "~\r\n", 3);
}

static void
editor_write_status(Buf *buf)
{
	size_t col_i;
	size_t len;

	color_begin(buf, COLOR_WHITE, COLOR_BLACK);
	/* Write file path */
	len = buf_writef(buf, " %s", editor.path);
	/* Fill colored empty space */
	for (col_i = len; col_i < editor.win_size.ws_col; col_i++)
		buf_write(buf, " ", 1);
	color_end(buf);
}
