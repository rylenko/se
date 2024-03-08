#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "color.h"
#include "cfg.h"
#include "cur.h"
#include "ed.h"
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
} ed;

/* Clears the screen. Then sets the cursor to the beginning of the screen. */
static void ed_clr_scr(Buf *buf);

/* Updates the size and checks that everything fits on the screen. */
static void ed_handle_sig_win_ch(int num);

/* Move cursor down. */
static void ed_mv_down(void);

/* Fixes cursor's coordinates. */
static void ed_fix_cur(void);

/* Move cursor left. */
static void ed_mv_left(void);

/* Move cursor right. */
static void ed_mv_right(void);

/* Move cursor up. */
static void ed_mv_up(void);

/* Quits the editor. */
static void ed_quit(void);

/*
Requests the size from the terminal and sets it in the appropriate field.

To update the window size after it has been changed, use the handler
`ed_handle_sig_win_ch`.
*/
static void ed_upd_win_size(void);

/* Writes cursor position including tabs. */
static void ed_write_cur(Buf *buf);

/* Write rows in the buffer. */
static void ed_write_rows(Buf *buf);

/* Write static in the buffer. */
static void ed_write_status(Buf *buf);

/* Converts mode to the string. */
static char *mode_str(Mode mode);

static void
ed_clr_scr(Buf *buf)
{
	unsigned short col_i;
	unsigned short row_i;
	/* Go home before clearing */
	term_go_home(buf);
	/* Clear the screen */
	for (row_i = 0; row_i < ed.win_size.ws_row; row_i++) {
		for (col_i = 0; col_i < ed.win_size.ws_col; col_i++) {
			buf_write(buf, " ", 1);
		}
	}
	/* Go home after clearing */
	term_go_home(buf);
}

static void
ed_fix_cur(void)
{
	/* Fix y coordinate. Must have after window resizing */
	ed.cur.y = MIN(ed.cur.y, ed.win_size.ws_row - 2);
	/* Fix x coordinate. Must have after window resizing and moving over lines */
	ed.cur.x = MIN(
		ed.cur.x,
		ed.rows.arr[ed.cur.y + ed.offset_row].len - ed.offset_col
	);
}

static void
ed_handle_sig_win_ch(int num)
{
	(void)num;
	ed_upd_win_size();
	ed_refresh_scr();
}

static void
ed_mv_down(void)
{
	/* Check that we need space to move down */
	if (ed.offset_row + ed.cur.y < ed.rows.cnt - 1) {
		if (ed.win_size.ws_row - 2 == ed.cur.y) {
			/* We are at the bottom of window */
			ed.offset_row++;
		} else {
			/* We are have enough space to move down on the screen */
			ed.cur.y++;
		}
	}
}

static void
ed_mv_left(void)
{
	if (0 == ed.cur.x) {
		if (ed.offset_col > 0) {
			/* We are at the top of window */
			ed.offset_col--;
		}
	} else {
		/* We are have enough space to move left on the screen */
		ed.cur.x--;
	}
}

static void
ed_mv_right(void)
{
	/* Get current row */
	const Row *row = &ed.rows.arr[ed.offset_row + ed.cur.y];
	/* Check that we need space to move right */
	if (ed.offset_col + ed.cur.x < row->len) {
		if (ed.win_size.ws_col - 1 == ed.cur.y) {
			/* We are at the right of window */
			ed.offset_col++;
		} else {
			/* We are have enough space to move right on the screen */
			ed.cur.x++;
		}
	}
}

static void
ed_mv_up(void)
{
	if (0 == ed.cur.y) {
		if (ed.offset_row > 0) {
			/* We are at the top of window */
			ed.offset_row--;
		}
	} else {
		/* We are have enough space to move up on the screen */
		ed.cur.y--;
	}
}

char
ed_need_to_quit(void)
{
	return ed.need_to_quit;
}

void
ed_open(const char *path)
{
	FILE *f;

	/* Initialize */
	ed.cur = cur_new(0, 0);
	ed.mode = MODE_NORM;
	ed.msg[0] = 0;
	ed.need_to_quit = 0;
	ed.offset_col = 0;
	ed.offset_row = 0;
	ed.path = str_clone(path);
	ed.rows = rows_alloc();
	/* Update window size and register the handler of window size changing */
	ed_upd_win_size();
	signal(SIGWINCH, ed_handle_sig_win_ch);

	/* Read rows from file  */
	if (!(f = fopen(path, "r"))) {
		err("Failed to open \"%s\":", path);
	}
	rows_read(&ed.rows, f);
	fclose(f);
}

static void
ed_quit(void)
{
	ed.need_to_quit = 1;
	/* Free memory */
	free(ed.path);
	rows_free(&ed.rows);
}

void
ed_refresh_scr(void)
{
	/* Allocate new buffer, hide cursor and clear the screen */
	Buf buf = buf_alloc();
	cur_hide(&buf);
	ed_clr_scr(&buf);

	/* Write content if we do not quit yet */
	if (!ed.need_to_quit) {
		/* Write main components */
		ed_write_rows(&buf);
		ed_write_status(&buf);

		/* Fix cursor and write it to buffer */
		ed_fix_cur();
		ed_write_cur(&buf);
	}

	/* Show cursor, flush and free the buffer */
	cur_show(&buf);
	term_flush(&buf);
	buf_free(buf);
}

static void
ed_upd_win_size(void)
{
	term_get_win_size(&ed.win_size);
}

static void
ed_write_cur(Buf *buf)
{
	unsigned short cont_i;
	size_t x = 0;
	const Row *row = &ed.rows.arr[ed.cur.y + ed.offset_row];
	/* Calculate tabs */
	for (
		cont_i = ed.offset_col;
		cont_i < ed.offset_col + ed.cur.x;
		cont_i++
	) {
		if (row->cont[cont_i] == '\t') {
			x += CFG_TAB_SIZE - x % CFG_TAB_SIZE - 1;
		}
		x++;
	}
	/* Write cursor */
	cur_write(cur_new(x, ed.cur.y), buf);
}

void
ed_wait_and_proc_key(void)
{
	char key;
	/* Assert that we do not need to quit */
	assert(!ed.need_to_quit);

	/* Wait key */
	key = term_wait_key();

	/* Process pressed key */
	switch (ed.mode) {
	case MODE_NORM:
		/* Normal mode keys */
		switch (key) {
		/* Move left */
		case KEY_H:
			ed_mv_left();
			break;
		/* Move down */
		case KEY_J:
			ed_mv_down();
			break;
		/* Move up */
		case KEY_K:
			ed_mv_up();
			break;
		/* Move right */
		case KEY_L:
			ed_mv_right();
			break;
		/* Quit */
		case KEY_CTRL_Q:
			ed_quit();
			break;
		/* Save */
		case KEY_CTRL_S:
			strcpy(ed.msg, MSG_SAVED);
			break;
		/* Switch to insert mode */
		case KEY_I:
			ed.mode = MODE_INS;
			break;
		}
		break;
	case MODE_INS:
		/* Insert mode keys */
		switch (key) {
		/* Switch to normal mode */
		case KEY_ESC:
			ed.mode = MODE_NORM;
			break;
		}
		break;
	}
}

static void
ed_write_rows(Buf *buf)
{
	const Row *row;
	unsigned short row_i;
	/* Assert that we do not need to quit */
	assert(!ed.need_to_quit);

	for (row_i = 0; row_i < ed.win_size.ws_row - 1; row_i++) {
		const size_t f_row_i = row_i + ed.offset_row;

		if (f_row_i >= ed.rows.cnt) {
			/* No row */
			buf_write(buf, "~\r\n", 3);
		} else {
			/* Write row */
			row = &ed.rows.arr[f_row_i];
			buf_write(buf, row->cont, MIN(ed.win_size.ws_col, row->len));
			buf_write(buf, "\r\n", 2);
		}
	}
}

static void
ed_write_status(Buf *buf)
{
	size_t col_i;
	size_t len = 0;
	color_begin(buf, COLOR_WHITE, COLOR_BLACK);

	/* Write base status */
	len += buf_writef(buf, " (%s) %s", mode_str(ed.mode), ed.path);

	/* Write message if exists */
	if (ed.msg[0]) {
		len += buf_writef(buf, ": %s", ed.msg);
		/* That is, the message will disappear after the next key */
		ed.msg[0] = 0;
	}

	/* Fill colored empty space */
	for (col_i = len; col_i < ed.win_size.ws_col; col_i++) {
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
