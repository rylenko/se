/* TODO: `Row *ed_get_cur_row(void)` */

#include <assert.h>
#include <libgen.h>
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
#include "math.h"
#include "mode.h"
#include "row.h"
#include "str_util.h"
#include "term.h"
#include "tok.h"

/* Length of message's buffer must be greater than all message lengths */
#define MSG_BUF_LEN (32)
#define MSG_SAVED ("The file has been saved.")

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

/* Move to begin of file. */
static void ed_mv_begin_of_f(void);

/* Move to begin of row. */
static void ed_mv_begin_of_row(void);

/* Move cursor down. */
static void ed_mv_down(void);

/* Move to end of file. */
static void ed_mv_end_of_f(void);

/* Move to end of row. */
static void ed_mv_end_of_row(void);

/* Fixes cursor's coordinates. */
static void ed_fix_cur(void);

/* Move cursor left. */
static void ed_mv_left(void);

/* Move to next token if exists. */
static void ed_mv_next_tok(void);

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
	const Row *row = &ed.rows.arr[ed.cur.y + ed.offset_row];
	size_t f_col_i = ed.offset_col + ed.cur.x;
	size_t col_diff;

	/* TODO check x coordinate after window resizing */
	/* Fix y coordinate. Must have after window resizing */
	ed.cur.y = MIN(ed.cur.y, ed.win_size.ws_row - 2);

	/* Fix x coordinate if current row does not has enough length */
	if (f_col_i > row->len) {
		col_diff = f_col_i - row->len;

		if (ed.cur.x < col_diff) {
			/* Return row on the screen */
			ed.offset_col -= col_diff - ed.cur.x;
			ed.cur.x = 0;
			/* Show last character of the row if exists */
			if (row->len > 0) {
				ed.offset_col--;
			}
		} else {
			/* Offset the cursor */
			ed.cur.x -= col_diff;
		}
	}
}

static void
ed_handle_sig_win_ch(int num)
{
	(void)num;
	ed_upd_win_size();
	ed_refresh_scr();
	ed_fix_cur();
}

static void
ed_mv_begin_of_f(void)
{
	ed.offset_row = 0;
	ed.cur.x = 0;
	ed.cur.y = 0;
}

static void
ed_mv_end_of_f(void)
{
	ed.offset_row = ed.rows.cnt - ed.win_size.ws_row + 1;
	ed.cur.x = 0;
	ed.cur.y = ed.win_size.ws_row - 2;
}

static void
ed_mv_begin_of_row(void)
{
	ed.offset_col = 0;
	ed.cur.x = 0;
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
	ed_fix_cur();
}

static void
ed_mv_end_of_row(void)
{
	const Row *row = &ed.rows.arr[ed.offset_row + ed.cur.y];
	if (row->len < ed.offset_col + ed.win_size.ws_col) {
		/* End of row on the screen */
		ed.cur.x = row->len - ed.offset_col;
	} else {
		/* Offset to see end of row on the screen */
		ed.offset_col = row->len - ed.win_size.ws_col + 1;
		ed.cur.x = ed.win_size.ws_col - 1;
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
ed_mv_next_tok(void)
{
	char *tok;
	size_t step;
	size_t f_col_i = ed.offset_col + ed.cur.x;
	/* Get string after cursor */
	const char *point = &ed.rows.arr[ed.offset_row + ed.cur.y].cont[f_col_i];
	/* TODO: check next line for the next token */
	if ((tok = tok_next(point)) != NULL) {
		step = tok - point;
		/* Check token on the screen */
		if (step + ed.cur.x < ed.win_size.ws_col) {
			ed.cur.x += step;
		} else {
			ed.offset_col = f_col_i + step - ed.win_size.ws_col + 1;
			ed.cur.x = ed.win_size.ws_col - 1;
		}
	}
}

static void
ed_mv_right(void)
{
	/* Get current row */
	const Row *row = &ed.rows.arr[ed.offset_row + ed.cur.y];
	/* Check that we need space to move right */
	if (ed.offset_col + ed.cur.x < row->len) {
		if (ed.win_size.ws_col - 1 == ed.cur.x) {
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
	ed_fix_cur();
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
	/* TODO: add empty line if there is not lines readed */
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
		ed_write_rows(&buf);
		ed_write_status(&buf);
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
		case KEY_BEGIN_OF_F:
			ed_mv_begin_of_f();
			break;
		case KEY_BEGIN_OF_ROW:
			ed_mv_begin_of_row();
			break;
		case KEY_DOWN:
			ed_mv_down();
			break;
		case KEY_END_OF_F:
			ed_mv_end_of_f();
			break;
		case KEY_END_OF_ROW:
			ed_mv_end_of_row();
			break;
		case KEY_INS_MODE:
			ed.mode = MODE_INS;
			break;
		case KEY_LEFT:
			ed_mv_left();
			break;
		case KEY_NEXT_TOK:
			ed_mv_next_tok();
			break;
		case KEY_QUIT:
			ed_quit();
			break;
		case KEY_RIGHT:
			ed_mv_right();
			break;
		case KEY_SAVE:
			strcpy(ed.msg, MSG_SAVED);
			break;
		case KEY_UP:
			ed_mv_up();
			break;
		}
		break;
	case MODE_INS:
		/* Insert mode keys */
		switch (key) {
		case KEY_NORM_MODE:
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
			if (row->len > ed.offset_col) {
				buf_write(
					buf,
					row->cont + ed.offset_col,
					MIN(ed.win_size.ws_col, row->len - ed.offset_col)
				);
			}
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
	len += buf_writef(
		buf,
		" [%s] %s (%zu, %zu)",
		mode_str(ed.mode),
		basename(ed.path),
		ed.offset_col + ed.cur.x + 1,
		ed.offset_row + ed.cur.y + 1
	);
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
