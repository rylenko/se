#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "color.h"
#include "cfg.h"
#include "cur.h"
#include "editor.h"
#include "err.h"
#include "key.h"
#include "math.h"
#include "row.h"
#include "str_util.h"
#include "term.h"

/* Length of message's buffer must be greater than all message lengths */
#define MSG_BUF_LEN (64)
#define MSG_SAVED ("The file has been saved.")

/* Editing mode */
typedef enum {
	MODE_INS,
	MODE_NORM,
} Mode;

/* Structure with editor parameters. */
static struct {
	Cur cur;
	char need_to_quit;
	Mode mode;
	char msg[MSG_BUF_LEN];
	size_t offset_col;
	size_t offset_row;
	char *path;
	Rows rows;
	struct winsize win_size;
} editor;

/* Clears the screen. Then sets the cursor to the beginning of the screen. */
static void editor_clr_scr(Buf *buf);

/* Updates the size and checks that everything fits on the screen. */
static void editor_handle_sig_win_ch(int num);

/* Move cursor down. */
static void editor_mv_down(void);

/* Fixes cursor's coordinates. */
static void editor_fix_cur(void);

/* Move cursor left. */
static void editor_mv_left(void);

/* Move cursor right. */
static void editor_mv_right(void);

/* Move cursor up. */
static void editor_mv_up(void);

/* Quits the editor. */
static void editor_quit(void);

/*
Requests the size from the terminal and sets it in the appropriate field.

To update the window size after it has been changed, use the handler
`editor_handle_sig_win_ch`.
*/
static void editor_upd_win_size(void);

/* Writes cursor position including tabs. */
static void editor_write_cur(Buf *buf);

/* Write rows in the buffer. */
static void editor_write_rows(Buf *buf);

/* Write static in the buffer. */
static void editor_write_status(Buf *buf);

/* Converts mode to the string. */
static char *mode_str(Mode mode);

static void
editor_clr_scr(Buf *buf)
{
	unsigned short col_i;
	unsigned short row_i;
	/* Go home before clearing */
	term_go_home(buf);
	/* Clear the screen */
	for (row_i = 0; row_i < editor.win_size.ws_row; row_i++) {
		for (col_i = 0; col_i < editor.win_size.ws_col; col_i++) {
			buf_write(buf, " ", 1);
		}
	}
	/* Go home after clearing */
	term_go_home(buf);
}

static void
editor_fix_cur(void)
{
	/* Fix y coordinate. Must have after window resizing */
	editor.cur.y = MIN(editor.cur.y, editor.win_size.ws_row - 2);
	/* Fix x coordinate. Must have after window resizing and moving over lines */
	editor.cur.x = MIN(
		editor.cur.x,
		editor.rows.arr[editor.cur.y + editor.offset_row].len - editor.offset_col
	);
}

static void
editor_handle_sig_win_ch(int num)
{
	(void)num;
	editor_upd_win_size();
	editor_refresh_scr();
}

static void
editor_mv_down(void)
{
	/* Check that we need space to move down */
	if (editor.offset_row + editor.cur.y < editor.rows.cnt - 1) {
		if (editor.win_size.ws_row - 2 == editor.cur.y) {
			/* We are at the bottom of window */
			editor.offset_row++;
		} else {
			/* We are have enough space to move down on the screen */
			editor.cur.y++;
		}
	}
}

static void
editor_mv_left(void)
{
	if (0 == editor.cur.x) {
		if (editor.offset_col > 0) {
			/* We are at the top of window */
			editor.offset_col--;
		}
	} else {
		/* We are have enough space to move left on the screen */
		editor.cur.x--;
	}
}

static void
editor_mv_right(void)
{
	/* Get current row */
	const Row *row = &editor.rows.arr[editor.offset_row + editor.cur.y];
	/* Check that we need space to move right */
	if (editor.offset_col + editor.cur.x < row->len) {
		if (editor.win_size.ws_col - 1 == editor.cur.y) {
			/* We are at the right of window */
			editor.offset_col++;
		} else {
			/* We are have enough space to move right on the screen */
			editor.cur.x++;
		}
	}
}

static void
editor_mv_up(void)
{
	if (0 == editor.cur.y) {
		if (editor.offset_row > 0) {
			/* We are at the top of window */
			editor.offset_row--;
		}
	} else {
		/* We are have enough space to move up on the screen */
		editor.cur.y--;
	}
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
	editor.cur = cur_new(0, 0);
	editor.mode = MODE_NORM;
	editor.msg[0] = 0;
	editor.need_to_quit = 0;
	editor.offset_col = 0;
	editor.offset_row = 0;
	editor.path = str_clone(path);
	editor.rows = rows_alloc();
	/* Update window size and register the handler of window size changing */
	editor_upd_win_size();
	signal(SIGWINCH, editor_handle_sig_win_ch);

	/* Read rows from file  */
	if (!(f = fopen(path, "r"))) {
		err("Failed to open \"%s\":", path);
	}
	rows_read(&editor.rows, f);
	fclose(f);
}

static void
editor_quit(void)
{
	editor.need_to_quit = 1;
	/* Free memory */
	free(editor.path);
	rows_free(&editor.rows);
}

void
editor_refresh_scr(void)
{
	/* Allocate new buffer, hide cursor and clear the screen */
	Buf buf = buf_alloc();
	cur_hide(&buf);
	editor_clr_scr(&buf);

	/* Write content if we do not quit yet */
	if (!editor.need_to_quit) {
		/* Write main components */
		editor_write_rows(&buf);
		editor_write_status(&buf);

		/* Fix cursor and write it to buffer */
		editor_fix_cur();
		editor_write_cur(&buf);
	}

	/* Show cursor, flush and free the buffer */
	cur_show(&buf);
	term_flush(&buf);
	buf_free(buf);
}

static void
editor_upd_win_size(void)
{
	term_get_win_size(&editor.win_size);
}

static void
editor_write_cur(Buf *buf)
{
	unsigned short cont_i;
	size_t x = 0;
	const Row *row = &editor.rows.arr[editor.cur.y + editor.offset_row];
	/* Calculate tabs */
	for (
		cont_i = editor.offset_col;
		cont_i < editor.offset_col + editor.cur.x;
		cont_i++
	) {
		if (row->cont[cont_i] == '\t') {
			x += CFG_TAB_SIZE - x % CFG_TAB_SIZE - 1;
		}
		x++;
	}
	/* Write cursor */
	cur_write(cur_new(x, editor.cur.y), buf);
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
		/* Move left */
		case KEY_H:
			editor_mv_left();
			break;
		/* Move down */
		case KEY_J:
			editor_mv_down();
			break;
		/* Move up */
		case KEY_K:
			editor_mv_up();
			break;
		/* Move right */
		case KEY_L:
			editor_mv_right();
			break;
		/* Quit */
		case KEY_CTRL_Q:
			editor_quit();
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
editor_write_rows(Buf *buf)
{
	const Row *row;
	unsigned short row_i;
	/* Assert that we do not need to quit */
	assert(!editor.need_to_quit);

	for (row_i = 0; row_i < editor.win_size.ws_row - 1; row_i++) {
		const size_t f_row_i = row_i + editor.offset_row;

		if (f_row_i >= editor.rows.cnt) {
			/* No row */
			buf_write(buf, "~\r\n", 3);
		} else {
			/* Write row */
			row = &editor.rows.arr[f_row_i];
			buf_write(buf, row->cont, MIN(editor.win_size.ws_col, row->len));
			buf_write(buf, "\r\n", 2);
		}
	}
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
	for (col_i = len; col_i < editor.win_size.ws_col; col_i++) {
		buf_write(buf, " ", 1);
	}
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
