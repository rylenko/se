#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "color.h"
#include "editor.h"
#include "err.h"
#include "key.h"
#include "term.h"

/* Length of message's buffer must be greater than all message lengths */
#define MSG_BUF_LEN (64)
#define MSG_SAVED ("The file has been saved.")

typedef enum {
	MODE_INS,
	MODE_NORM,
} Mode;

/* Structure with editor parameters. */
static struct {
	char need_to_quit;
	Mode mode;
	char msg[MSG_BUF_LEN];
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

/* Converts mode to the string. */
static char *mode_str(Mode mode);

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
	char *line;
	size_t line_len;

	/* Initialize */
	editor.mode = MODE_NORM;
	editor.msg[0] = 0;
	editor.need_to_quit = 0;
	/* TODO: should we need to copy it? */
	editor.path = path;

	/* Update window size and register the handler of window size changing */
	editor_update_win_size();
	signal(SIGWINCH, editor_handle_sig_win_ch);

	/* Open the file  */
	if (!(f = fopen(path, "r")))
		err("Failed to open \"%s\":", path);

	/* Read lines */
	while (read_line(f, &line, &line_len)) {
		editor_insert(editor.rows_count, line, line_len);
		free(line);
	}

	/* Deallocate lose the file after read */
	free(line);
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
	char key;
	/* Assert that we do not need to quit */
	assert(!editor.need_to_quit);

	/* Wait key */
	key = term_wait_key_press();

	/* Process pressed key */
	switch (editor.mode) {
	case MODE_NORM:
		/* Normal mode keys */
		switch (key) {
		/* Quit */
		case KEY_CTRL_Q:
			editor.need_to_quit = 1;
			break;
		/* Save */
		case KEY_CTRL_S:
			strcpy(editor.msg, MSG_SAVED);
			break;
		/* Switch to insert mode */
		case KEY_I:
			editor.mode = MODE_INS;
			break;
		}
		break;
	case MODE_INS:
		/* Insert mode keys */
		switch (key) {
		/* Switch to normal mode */
		case KEY_ESC:
			editor.mode = MODE_NORM;
			break;
		}
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
	size_t len = 0;
	color_begin(buf, COLOR_WHITE, COLOR_BLACK);

	/* Write base status */
	len += buf_writef(buf, " (%s) %s", mode_str(editor.mode), editor.path);

	/* Write message if exists */
	if (editor.msg[0]) {
		len += buf_writef(buf, ": %s", editor.msg);
		/* That is, the message will disappear after the next key */
		editor.msg[0] = 0;
	}

	/* Fill colored empty space */
	for (col_i = len; col_i < editor.win_size.ws_col; col_i++)
		buf_write(buf, " ", 1);
	color_end(buf);
}

static char*
mode_str(Mode mode)
{
	switch (mode) {
	case MODE_INS:
		return "INSERT";
	case MODE_NORM:
		return "NORMAL";
	default:
		return NULL;
	}
}
