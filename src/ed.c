#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "buf.h"
#include "cfg.h"
#include "cur.h"
#include "ed.h"
#include "err.h"
#include "math.h"
#include "mode.h"
#include "raw_color.h"
#include "row.h"
#include "str_util.h"
#include "term.h"
#include "tok.h"

/* Length of message's buffer must be greater than all message lengths */
#define MSG_BUF_LEN (64)
#define MSG_SAVED ("The file has been saved.")
#define MSG_QUIT_PRESSES_REM_FMT ( \
	"There are unsaved changes. Presses remain to quit: %hhu." \
)

#define STAT_COORDS_BUF_LEN (32)

/* Structure with editor parameters. */
static struct {
	Cur cur;
	Mode mode;
	char msg[MSG_BUF_LEN];
	/* Maximum of `size_t` if not set */
	size_t num_input;
	size_t offset_col;
	size_t offset_row;
	char *path;
	unsigned char quit_presses_rem;
	Rows rows;
	struct winsize win_size;
} ed;

/* Fixes cursor's coordinates. */
static void ed_fix_cur(void);

/* Gets current row. */
static Row *ed_get_curr_row(void);

/* Updates the size and checks that everything fits on the screen. */
static void ed_handle_sig_win_ch(int num);

/* Input number. */
static void ed_input_num(unsigned char digit);

/* Inserts new row below the cursor. */
static void ed_ins_row_below(void);

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

/* Move to prepared row's index. */
static void ed_mv_input_row(void);

/* Move cursor left. */
static void ed_mv_left(void);

/* Move to next token if exists. */
static void ed_mv_next_tok(void);

/* Move to previous token if exists. */
static void ed_mv_prev_tok(void);

/* Move cursor right. */
static void ed_mv_right(void);

/* Move to row by its index. */
static void ed_mv_row(size_t idx);

/* Move cursor up. */
static void ed_mv_up(void);

/* Quits the editor. */
static void ed_try_quit(void);

/* Saves file. */
static void ed_save(void);

/* Set message. */
static void ed_set_msg(const char *fmt, ...);

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
static void ed_write_stat(Buf *buf);

static void
ed_fix_cur(void)
{
	const Row *row = ed_get_curr_row();
	size_t f_col_i = ed.offset_col + ed.cur.x;
	size_t col_diff;

	/* Clamp cursor on the screen */
	ed.cur.y = MIN(ed.cur.y, ed.win_size.ws_row - 2);
	ed.cur.x = MIN(ed.cur.x, ed.win_size.ws_col - 1);

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

static Row*
ed_get_curr_row(void)
{
	return &ed.rows.arr[ed.cur.y + ed.offset_row];
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
	ed.offset_col = 0;
	ed.offset_row = 0;
	ed.cur.x = 0;
	ed.cur.y = 0;
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
ed_mv_end_of_f(void)
{
	ed_mv_row(ed.rows.cnt - 1);
}

static void
ed_mv_end_of_row(void)
{
	const Row *row = ed_get_curr_row();
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
ed_mv_input_row(void)
{
	if (ed.num_input != SIZE_MAX) {
		ed_mv_row(ed.num_input);
		ed.num_input = SIZE_MAX;
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
	/* Find next token */
	const Row *row = ed_get_curr_row();
	size_t f_col_i = ed.offset_col + ed.cur.x;
	size_t len = row->len - f_col_i;
	size_t tok_i = tok_next(row->cont + f_col_i, row->len - f_col_i);

	if (tok_i < len) {
		/* Check token on the screen */
		if (tok_i + ed.cur.x < ed.win_size.ws_col) {
			ed.cur.x += tok_i;
		} else {
			ed.offset_col = f_col_i + tok_i - ed.win_size.ws_col + 1;
			ed.cur.x = ed.win_size.ws_col - 1;
		}
	}
}

static void
ed_mv_prev_tok(void)
{
	/* Find previous token */
	size_t f_col_i = ed.offset_col + ed.cur.x;
	size_t tok_i = tok_rnext(ed_get_curr_row()->cont, f_col_i);

	if (tok_i < f_col_i) {
		/* Check token on the screen */
		if (tok_i >= ed.offset_col) {
			ed.cur.x = tok_i - ed.offset_col;
		} else {
			ed.offset_col = tok_i;
			ed.cur.x = 1;
		}
	}
}

static void
ed_mv_right(void)
{
	/* Check that we need space to move right */
	if (ed.offset_col + ed.cur.x < ed_get_curr_row()->len) {
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
ed_mv_row(size_t idx)
{
	/* Remove offsets by x */
	ed.cur.x = 0;
	ed.offset_col = 0;

	/* Clamp index and move */
	idx = MIN(idx, ed.rows.cnt - 1);
	if (idx + 1 < ed.win_size.ws_row) {
		/* Row on initial screen without offset */
		ed.offset_row = 0;
		ed.cur.y = idx;
	} else {
		/* End of file not on the screen */
		ed.offset_row = idx + 2 - ed.win_size.ws_row;
		ed.cur.y = ed.win_size.ws_row - 2;
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
	return ed.quit_presses_rem == 0;
}

void
ed_open(const char *path)
{
	FILE *f;

	/* Initialize */
	ed.cur = cur_new(0, 0);
	ed.mode = MODE_NORM;
	ed.msg[0] = 0;
	ed.num_input = SIZE_MAX;
	ed.offset_col = 0;
	ed.offset_row = 0;
	ed.path = str_clone(path);
	ed.quit_presses_rem = 1;
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

	/* Add empty row if there is no rows */
	if (ed.rows.cnt == 0) {
		rows_ins(&ed.rows, 0, row_empty());
	}
}

static void
ed_input_num(unsigned char digit)
{
	assert(digit < 10);
	if (ed.num_input == SIZE_MAX) {
		/* First digit in input */
		ed.num_input = 0;
	}
	if (SIZE_MAX / 10 - digit <= ed.num_input) {
		/* Too big. `SIZE_MAX` is flag that there is no pending index */
		ed.num_input = SIZE_MAX - 1;
	} else {
		/* Append digit to pending index */
		ed.num_input *= 10;
		ed.num_input += digit;
	}
}

static void
ed_ins_row_below(void)
{
	/* Check cursor at the bottom of the screen */
	if (ed.cur.y == ed.win_size.ws_row - 2) {
		ed.offset_row++;
	} else {
		ed.cur.y++;
	}
	/* Insert new empty row */
	rows_ins(&ed.rows, ed.offset_row + ed.cur.y, row_empty());
	ed.quit_presses_rem = CFG_QUIT_PRESSES_REM_WITHOUT_SAVE_AFTER_CHANGES;
}

static void
ed_try_quit(void)
{
	ed.quit_presses_rem--;
	if (ed_need_to_quit()) {
		/* Free memory */
		free(ed.path);
		rows_free(&ed.rows);
	} else {
		ed_set_msg(MSG_QUIT_PRESSES_REM_FMT, ed.quit_presses_rem);
	}
}

void
ed_refresh_scr(void)
{
	/* Allocate new buffer */
	Buf buf = buf_alloc();
	if (ed_need_to_quit()) {
		/* Clear the screen before quit */
		term_clr_scr(&buf);
	} else {
		/* Hide cursor and go to start of the screen */
		cur_hide(&buf);
		term_go_home(&buf);
		/* Write content */
		ed_write_rows(&buf);
		ed_write_stat(&buf);
		ed_write_cur(&buf);
		/* Show the cursor */
		cur_show(&buf);
	}
	/* Flush and free the buffer */
	term_flush(&buf);
	buf_free(buf);
}

static void
ed_save(void)
{
	size_t row_i;
	FILE *f;
	Row *row;

	/* Open file */
	if (!(f = fopen(ed.path, "w"))) {
		err("Failed to open %s for save.", ed.path);
	}
	/* Write rows */
	for (row_i = 0; row_i < ed.rows.cnt; row_i++) {
		row = &ed.rows.arr[row_i];
		/* Write row's content and newline character */
		/* TODO: Check errors */
		fwrite(row->cont, sizeof(char), row->len, f);
		fputc('\n', f);
	}
	/* Flush and close file */
	fflush(f);
	fclose(f);
	/* Set quit presses and set message */
	ed.quit_presses_rem = 1;
	ed_set_msg(MSG_SAVED);
}

static void
ed_set_msg(const char *fmt, ...)
{
	/* Collect arguments and print formatted message */
	va_list args;
	va_start(args, fmt);
	vsnprintf(ed.msg, sizeof(ed.msg), fmt, args);
	va_end(args);
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
	const Row *row = ed_get_curr_row();

	for (
		cont_i = ed.offset_col;
		cont_i < ed.offset_col + ed.cur.x;
		cont_i++
	) {
		/* Calculate tab offset */
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
	assert(!ed_need_to_quit());

	/* Wait key */
	key = term_wait_key();

	/* Process pressed key */
	switch (ed.mode) {
	case MODE_NORM:
		/* Normal mode keys */
		switch (key) {
		case CFG_KEY_INS_ROW_BELOW:
			ed_ins_row_below();
			break;
		case CFG_KEY_MODE_INS:
			ed.mode = MODE_INS;
			break;
		case CFG_KEY_MV_BEGIN_OF_F:
			ed_mv_begin_of_f();
			break;
		case CFG_KEY_MV_BEGIN_OF_ROW:
			ed_mv_begin_of_row();
			break;
		case CFG_KEY_MV_DOWN:
			ed_mv_down();
			break;
		case CFG_KEY_MV_END_OF_F:
			ed_mv_end_of_f();
			break;
		case CFG_KEY_MV_END_OF_ROW:
			ed_mv_end_of_row();
			break;
		case CFG_KEY_MV_LEFT:
			ed_mv_left();
			break;
		case CFG_KEY_MV_NEXT_TOK:
			ed_mv_next_tok();
			break;
		case CFG_KEY_MV_PREV_TOK:
			ed_mv_prev_tok();
			break;
		case CFG_KEY_MV_RIGHT:
			ed_mv_right();
			break;
		case CFG_KEY_MV_ROW:
			ed_mv_input_row();
			break;
		case CFG_KEY_MV_UP:
			ed_mv_up();
			break;
		case CFG_KEY_SAVE:
			ed_save();
			break;
		case CFG_KEY_TRY_QUIT:
			ed_try_quit();
			break;
		}
		/* Number input */
		if (raw_key_is_digit(key)) {
			ed_input_num(raw_key_to_digit(key));
		} else {
			ed.num_input = SIZE_MAX;
		}
		break;
	case MODE_INS:
		/* Insert mode keys */
		switch (key) {
		case CFG_KEY_MODE_NORM:
			ed.mode = MODE_NORM;
			break;
		}
		break;
	}
}

static void
ed_write_rows(Buf *buf)
{
	size_t f_row_i;
	const Row *row;
	unsigned short row_i;

	for (row_i = 0; row_i < ed.win_size.ws_row - 1; row_i++) {
		term_clr_row_on_right(buf);

		/* Write row */
		f_row_i = row_i + ed.offset_row;
		if (f_row_i >= ed.rows.cnt) {
			/* No row */
			buf_write(buf, "~", 1);
		} else {
			row = &ed.rows.arr[f_row_i];
			/* This condition also skips empty rows */
			if (row->len > ed.offset_col) {
				buf_write(
					buf,
					row->cont + ed.offset_col,
					MIN(ed.win_size.ws_col, row->len - ed.offset_col)
				);
			}
		}
		buf_write(buf, "\r\n", 2);
	}
}

static void
ed_write_stat(Buf *buf)
{
	size_t col_i;
	size_t left_len;
	size_t right_len;
	char coords[STAT_COORDS_BUF_LEN];

	/* Clear row on the right and begin colored output */
	term_clr_row_on_right(buf);
	raw_color_begin(
		buf,
		(RawColor)CFG_COLOR_STAT_BG,
		(RawColor)CFG_COLOR_STAT_FG
	);

	/* Write base status to buffer */
	left_len = buf_writef(
		buf,
		" [%s] %s",
		mode_str(ed.mode),
		ed.path
	);
	/* Write message to buffer if exists */
	if (ed.msg[0]) {
		left_len += buf_writef(buf, ": %s", ed.msg);
		/* That is, the message will disappear after the next key */
		ed.msg[0] = 0;
	}
	/* Format coordinates before colored empty space */
	right_len = snprintf(
		coords,
		sizeof(coords),
		"%zu, %zu ",
		ed.offset_col + ed.cur.x,
		ed.offset_row + ed.cur.y
	);
	/* Fill colored empty space */
	for (col_i = left_len + right_len; col_i < ed.win_size.ws_col; col_i++) {
		buf_write(buf, " ", 1);
	}
	/* Write coordinates */
	buf_write(buf, coords, right_len);

	raw_color_end(buf);
}
