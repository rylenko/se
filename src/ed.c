/* TODO: Add local clipboard. Use it in functions. */
/* TODO: Use linked list for rows array and row's content parts */
/* TODO: Integrate repetition of keys into handlers */
/* TODO: Undo operations. Also rename "del" to "remove" where needed */
/* TODO: Maybe split ed.c into ed_init.c, ed_stat.c, ed_key.c, etc.? */
/* TODO: Support huge files. */
/* TODO: Xclip patch to use with local clipboard */

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "buf.h"
#include "cfg.h"
#include "cur.h"
#include "ed.h"
#include "macros.h"
#include "math.h"
#include "mode.h"
#include "raw_color.h"
#include "raw_key.h"
#include "row.h"
#include "str_util.h"
#include "term.h"
#include "word.h"

/* Length of message's array must be greater than all message lengths */
static const char *const msg_del_only_one_row = \
	"It is forbidden to delete only one row.";
static const char *const msg_saved_fmt = "%zu bytes saved.";
static const char *const msg_save_fail_fmt = "Failed to save: %s.";
static const char *const msg_quit_presses_rem_fmt = \
	"There are unsaved changes. Presses remain to quit: %hhu.";

enum {
	MSG_ARR_LEN = 64,
};

/* Structure with editor parameters. */
static struct {
	Cur cur;
	char is_dirty;
	Mode mode;
	char msg[MSG_ARR_LEN];
	/* Maximum of `size_t` if not set */
	size_t num_input;
	size_t offset_col;
	size_t offset_row;
	char *path;
	unsigned char quit_presses_rem;
	/* There is always at least 1 row */
	Rows rows;
	struct winsize win_size;
} ed;

/* Breaks the row into two rows. */
static void ed_break_row(void);

/* Deletes current character. */
static void ed_del(void);

/* Deletes current row. */
static void ed_del_row(size_t times);

/* Fixes cursor's coordinates. */
static void ed_fix_cur(void);

/* Gets current row. Use it carefully with reallocations. */
static Row *ed_get_curr_row(void);

/* Updates the size and checks that everything fits on the screen. */
static void ed_handle_sig_win_ch(int num);

/* Input number. */
static void ed_input_num(const unsigned char digit);

/* Inserts a character. */
static void ed_ins(char ch);

/* Inserts new row below the cursor and switches to inserting mode. */
static void ed_ins_row_below(void);

/* Inserts new row on top of the cursor and switches to inserting mode. */
static void ed_ins_row_top(void);

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

/* Move cursor left. */
static void ed_mv_left(void);

/* Move to next word if exists. */
static void ed_mv_next_word(size_t times);

/* Move to previous word if exists. */
static void ed_mv_prev_word(size_t times);

/* Move cursor right. */
static void ed_mv_right(void);

/* Move to row by its index. */
static void ed_mv_row(size_t idx);

/* Move cursor up. */
static void ed_mv_up(void);

/* State updater of file change. */
static void ed_on_f_ch(void);

/* Processes arrow key sequence. */
static void ed_proc_arrow_key(char key);

/* Processes inserting key. */
static void ed_proc_ins_key(char key);

/* Processes key sequence. Useful if single key press is several `char`s. */
static void ed_proc_key_seq(const char *key_seq, const size_t len);

/* Processes normal key. */
static void ed_proc_norm_key(char key);

/* Quits the editor. */
static void ed_try_quit(void);

/* Saves file. Saves to opened file if argument is `NULL`. */
static void ed_save(const char *path);

/* Saves file to spare dir. */
static void ed_save_to_spare_dir(void);

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

/* Writes status to the buffer. */
static void ed_write_stat(Buf *buf);

/* Writes left part of status to the buffer. */
static size_t ed_write_stat_left(Buf *buf);

/* Writes right part of status to the buffer. */
static void ed_write_stat_right(Buf *buf, size_t left_len);


static void
ed_break_row(void)
{
	/* Break the row */
	rows_break(&ed.rows, ed.offset_row + ed.cur.y, ed.offset_col + ed.cur.x);
	/* Remove x offset and move down */
	ed.cur.x = 0;
	ed.offset_col = 0;
	if (ed.cur.y + 2 == ed.win_size.ws_row) {
		ed.offset_row++;
	} else {
		ed.cur.y++;
	}
}

void
ed_deinit(void)
{
	term_disable_raw_mode();
}

static void
ed_del(void)
{
	size_t f_col_i = ed.offset_col + ed.cur.x;
	if (0 == f_col_i) {
		/* TODO: union current and previous rows if current is not first */
	} else {
		/* Delete character */
		row_del(&ed.rows.arr[ed.offset_row + ed.cur.y], f_col_i - 1);
		/* Shift cursor */
		if (0 == ed.cur.x) {
			ed.offset_col--;
		} else {
			ed.cur.x--;
		}
		ed_on_f_ch();
	}
}

static void
ed_del_row(size_t times)
{
	if (1 == ed.rows.cnt) {
		ed_set_msg(msg_del_only_one_row);
	} else if (times > 0) {
		/* Get real repeat times */
		times = MIN(ed.rows.cnt - ed.offset_row - ed.cur.y, times);
		/* The file must contain at least one row */
		if (times == ed.rows.cnt) {
			times--;
		}
		/* Remove x offsets and delete the row */
		ed.offset_col = 0;
		ed.cur.x = 0;
		/* Delete */
		while (times-- > 0) {
			rows_del(&ed.rows, ed.offset_row + ed.cur.y);
		}
		ed_on_f_ch();
	}
}

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
	return &ed.rows.arr[ed.offset_row + ed.cur.y];
}

static void
ed_handle_sig_win_ch(int num)
{
	(void)num;
	ed_upd_win_size();
	ed_refresh_scr();
	ed_fix_cur();
}

void
ed_init(const int ifd, const int ofd)
{
	term_init(ifd, ofd);
	term_enable_raw_mode();
}

static void
ed_input_num(const unsigned char digit)
{
	assert(digit < 10);
	if (SIZE_MAX == ed.num_input) {
		/* Prepare for first digit in input */
		ed.num_input = 0;
	}
	if ((SIZE_MAX - digit) / 10 <= ed.num_input) {
		/* Too big. `SIZE_MAX` is flag that there is no pending number */
		ed.num_input = SIZE_MAX;
	} else {
		/* Append digit to pending index */
		ed.num_input *= 10;
		ed.num_input += digit;
	}
}

static void
ed_ins(char ch)
{
	/* Insert character */
	row_ins(&ed.rows.arr[ed.offset_row + ed.cur.y], ed.offset_col + ed.cur.x, ch);
	/* Shift cursor */
	if (ed.cur.x + 1 == ed.win_size.ws_col) {
		ed.offset_col++;
	} else {
		ed.cur.x++;
	}
	ed_on_f_ch();
}

static void
ed_ins_row_below(void)
{
	/* Remove x offsets */
	ed.offset_col = 0;
	ed.cur.x = 0;
	/* Check cursor at the bottom of the screen */
	if (ed.win_size.ws_row - 2 == ed.cur.y) {
		ed.offset_row++;
	} else {
		ed.cur.y++;
	}
	/* Insert new empty row */
	rows_ins(&ed.rows, ed.offset_row + ed.cur.y, row_empty());
	/* Handle file change and switch to inserting mode */
	ed_on_f_ch();
	ed.mode = MODE_INS;
}

static void
ed_ins_row_top(void)
{
	/* Remove x offsets */
	ed.offset_col = 0;
	ed.cur.x = 0;
	/* Insert new empty row */
	rows_ins(&ed.rows, ed.offset_row + ed.cur.y, row_empty());
	/* Offset cursor if we at the end of screen */
	if (ed.win_size.ws_row - 2 == ed.cur.y) {
		ed.offset_row++;
		ed.cur.y--;
	}
	/* Handle file change and switch to inserting mode */
	ed_on_f_ch();
	ed.mode = MODE_INS;
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
ed_mv_left(void)
{
	if (0 == ed.cur.x) {
		/* Check that we are at the left of window */
		if (ed.offset_col > 0) {
			ed.offset_col--;
		} else {
			/* TODO: move to previous row */
		}
	} else {
		/* We are have enough space to move left on the screen */
		ed.cur.x--;
	}
}

static void
ed_mv_next_word(size_t times)
{
	/* Find next word */
	const Row *row = ed_get_curr_row();
	size_t f_col_i;
	size_t word_i;

	while (times-- > 0) {
		/* Find next word */
		f_col_i = ed.offset_col + ed.cur.x;
		word_i = word_next(row->cont + f_col_i, row->len - f_col_i);
		/* Check word on the screen */
		if (word_i + ed.cur.x < ed.win_size.ws_col) {
			ed.cur.x += word_i;
		} else {
			ed.offset_col = f_col_i + word_i - ed.win_size.ws_col + 1;
			ed.cur.x = ed.win_size.ws_col - 1;
		}
		/* End of row */
		if (word_i + f_col_i == row->len) {
			break;
		}
	}
}

static void
ed_mv_prev_word(size_t times)
{
	size_t f_col_i;
	size_t word_i;

	while (times-- > 0) {
		/* Find previous word */
		f_col_i = ed.offset_col + ed.cur.x;
		word_i = word_rnext(ed_get_curr_row()->cont, f_col_i);
		/* Check word on the screen */
		if (word_i >= ed.offset_col) {
			ed.cur.x = word_i - ed.offset_col;
		} else {
			ed.offset_col = word_i;
			ed.cur.x = 1;
		}
		/* Start of row */
		if (word_i == 0) {
			break;
		}
	}
}

static void
ed_mv_right(void)
{
	if (ed.offset_col + ed.cur.x < ed_get_curr_row()->len) {
		if (ed.win_size.ws_col - 1 == ed.cur.x) {
			/* We are at the right of window */
			ed.offset_col++;
		} else {
			/* We are have enough space to move right on the screen */
			ed.cur.x++;
		}
	} else {
		/* TODO: move to previous row if exists */
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

static void
ed_on_f_ch(void)
{
	ed.is_dirty = 1;
	ed.quit_presses_rem = CFG_QUIT_PRESSES_REM_AFT_CH;
}

void
ed_open(const char *path)
{
	FILE *f;

	/* Set default for file */
	ed.cur = cur_new(0, 0);
	ed.is_dirty = 0;
	ed.mode = MODE_NORM;
	ed.msg[0] = 0;
	ed.num_input = SIZE_MAX;
	ed.offset_col = 0;
	ed.offset_row = 0;
	ed.path = str_clone(path);
	ed.quit_presses_rem = 1;
	ed.rows = rows_new();
	/* Update window size and register the handler of window size changing */
	ed_upd_win_size();
	if (signal(SIGWINCH, ed_handle_sig_win_ch) == SIG_ERR) {
		err(EXIT_FAILURE, "Failed to set window resize handler");
	}

	/* Read rows from file  */
	if (NULL == (f = fopen(path, "r"))) {
		err(EXIT_FAILURE, "Failed to open to read");
	}
	rows_read(&ed.rows, f);
	if (fclose(f) == EOF) {
		err(EXIT_FAILURE, "Failed to close readed file");
	}

	/* Add empty row if there is no rows */
	if (ed.rows.cnt == 0) {
		rows_ins(&ed.rows, 0, row_empty());
	}
}

static void
ed_proc_arrow_key(char key)
{
	/* Repititon times */
	size_t times = SIZE_MAX == ed.num_input ? 1 : ed.num_input;
	switch (key) {
	case 'A':
		REPEAT(times, ed_mv_up());
		return;
	case 'B':
		REPEAT(times, ed_mv_down());
		return;
	case 'C':
		REPEAT(times, ed_mv_right());
		return;
	case 'D':
		REPEAT(times, ed_mv_left());
		return;
	}
}

static void
ed_proc_ins_key(char key)
{
	switch (key) {
	/* Breaks current row */
	case CFG_KEY_BREAK_ROW:
		ed_break_row();
		return;
	/* Delete current character */
	case CFG_KEY_DEL:
		ed_del();
		return;
	/* Switch to normal mode */
	case CFG_KEY_MODE_NORM:
		ed.mode = MODE_NORM;
		return;
	}
	/* Check key is printable and insert */
	if (isprint(key)) {
		ed_ins(key);
	}
}

static void
ed_proc_key_seq(const char *key_seq, const size_t len)
{
	/* Arrows */
	if (
		len == 3
		&& RAW_KEY_ESC == key_seq[0]
		&& '[' == key_seq[1]
		&& 'A' <= key_seq[2]
		&& key_seq[2] <= 'D'
	) {
		ed_proc_arrow_key(key_seq[2]);
	}
}

static void
ed_proc_norm_key(char key)
{
	/* Repititon times */
	size_t repeat_times = SIZE_MAX == ed.num_input ? 1 : ed.num_input;

	/* Handle number input */
	if (raw_key_is_digit(key)) {
		ed_input_num(raw_key_to_digit(key));
		return;
	} else {
		ed.num_input = SIZE_MAX;
	}
	/* Other keys */
	switch (key) {
	case CFG_KEY_DEL_ROW:
		ed_del_row(repeat_times);
		return;
	case CFG_KEY_INS_ROW_BELOW:
		REPEAT(repeat_times, ed_ins_row_below());
		return;
	case CFG_KEY_INS_ROW_TOP:
		REPEAT(repeat_times, ed_ins_row_top());
		return;
	case CFG_KEY_MODE_INS:
		ed.mode = MODE_INS;
		return;
	case CFG_KEY_MV_TO_BEGIN_OF_F:
		ed_mv_begin_of_f();
		return;
	case CFG_KEY_MV_TO_BEGIN_OF_ROW:
		ed_mv_begin_of_row();
		return;
	case CFG_KEY_MV_DOWN:
		REPEAT(repeat_times, ed_mv_down());
		return;
	case CFG_KEY_MV_TO_END_OF_F:
		ed_mv_end_of_f();
		return;
	case CFG_KEY_MV_TO_END_OF_ROW:
		ed_mv_end_of_row();
		return;
	case CFG_KEY_MV_LEFT:
		REPEAT(repeat_times, ed_mv_left());
		return;
	case CFG_KEY_MV_TO_NEXT_WORD:
		ed_mv_next_word(repeat_times);
		return;
	case CFG_KEY_MV_TO_PREV_WORD:
		ed_mv_prev_word(repeat_times);
		return;
	case CFG_KEY_MV_RIGHT:
		REPEAT(repeat_times, ed_mv_right());
		return;
	case CFG_KEY_MV_UP:
		REPEAT(repeat_times, ed_mv_up());
		return;
	case CFG_KEY_SAVE:
		ed_save(NULL);
		return;
	case CFG_KEY_SAVE_TO_SPARE_DIR:
		ed_save_to_spare_dir();
		return;
	case CFG_KEY_TRY_QUIT:
		ed_try_quit();
		return;
	}
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
		ed_set_msg(msg_quit_presses_rem_fmt, ed.quit_presses_rem);
	}
}

void
ed_refresh_scr(void)
{
	/* Allocate new buffer and go to start of the screen */
	Buf buf = buf_alloc();
	term_go_home(&buf);
	if (ed_need_to_quit()) {
		/* Clear the screen before quit */
		term_clr_scr(&buf);
	} else {
		/* Hide cursor */
		cur_hide(&buf);
		/* Write content */
		ed_write_rows(&buf);
		ed_write_stat(&buf);
		ed_write_cur(&buf);
		/* Show the cursor */
		cur_show(&buf);
	}
	/* Flush and free the buffer */
	term_flush(&buf);
	buf_free(&buf);
}

static void
ed_save(const char *path)
{
	FILE *f;
	size_t len;

	/* Open file, write rows, flush and close file  */
	if (NULL == (f = fopen(path ? path : ed.path, "w"))) {
		ed_set_msg(msg_save_fail_fmt, strerror(errno));
		return;
	}
	len = rows_write(&ed.rows, f);
	if (fflush(f) == EOF) {
		err(EXIT_FAILURE, "Failed to flush saved file");
	} else if (fclose(f) == EOF) {
		err(EXIT_FAILURE, "Failed to close saved file");
	}
	/* Remove dirty flag and set message */
	ed.is_dirty = 0;
	ed.quit_presses_rem = 1;
	ed_set_msg(msg_saved_fmt, len);
}

/* Saves file to spare dir. */
static void
ed_save_to_spare_dir(void)
{
	time_t utc;
	struct tm *local;
	char date[15];
	char path[CFG_SPARE_SAVE_PATH_MAX_LEN] = {0};

	/* Get date */
	if ((utc = time(NULL)) == (time_t) - 1) {
		err(EXIT_FAILURE, "Failed to get time to save to spare dir");
	} else if (NULL == (local = localtime(&utc))) {
		err(EXIT_FAILURE, "Failed to get local time to save to spare dir");
	} else if (strftime(date, sizeof(date), "%m-%d_%H-%M-%S", local) == 0) {
		errx(EXIT_FAILURE, "Failed to convert time to string.");
	}
	/* Build full path and save */
	snprintf(
		path,
		sizeof(path),
		"%s/%s_%s",
		cfg_spare_save_dir,
		basename(ed.path),
		date
	);
	ed_save(path);
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
	char key_seq[3];
	size_t key_seq_len;
	/* Assert that we do not need to quit */
	assert(!ed_need_to_quit());
	/* Wait key sequence */
	key_seq_len = term_wait_key_seq(key_seq, sizeof(key_seq));
	/* Process entire key sequence if there is more than one `char` */
	if (key_seq_len > 1) {
		ed_proc_key_seq(key_seq, key_seq_len);
		return;
	}
	/* Process single `char` key press with specific mode */
	switch (ed.mode) {
	case MODE_NORM:
		ed_proc_norm_key(key_seq[0]);
		return;
	case MODE_INS:
		ed_proc_ins_key(key_seq[0]);
		return;
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
	size_t left_len;
	/* Clear row, begin color, write parts and end color */
	term_clr_row_on_right(buf);
	raw_color_begin(
		buf,
		(RawColor)CFG_COLOR_STAT_BG,
		(RawColor)CFG_COLOR_STAT_FG
	);
	left_len = ed_write_stat_left(buf);
	ed_write_stat_right(buf, left_len);
	raw_color_end(buf);
}

static size_t
ed_write_stat_left(Buf *buf)
{
	size_t len;
	/* Write base status to buffer */
	len = buf_writef(buf, " [%s] %s", mode_str(ed.mode), basename(ed.path));
	/* Add mark if file is dirty */
	if (ed.is_dirty) {
		len += buf_write(buf, " [+]", 4);
	}
	/* Write message to buffer if exists */
	if (ed.msg[0]) {
		len += buf_writef(buf, ": %s", ed.msg);
		ed.msg[0] = 0;
	}
	return len;
}

static void
ed_write_stat_right(Buf *buf, size_t left_len)
{
	size_t col_i;
	char coords[32];
	size_t coords_len;
	char num_input[32];
	size_t num_input_len = 0;

	/* Prepare number input if active */
	if (ed.num_input != SIZE_MAX) {
		num_input_len = snprintf(
			num_input,
			sizeof(num_input),
			"%zu << ",
			ed.num_input
		);
	}
	/* Prepare coordinates */
	coords_len = snprintf(
		coords,
		sizeof(coords),
		"%zu, %zu ",
		ed.offset_col + ed.cur.x,
		ed.offset_row + ed.cur.y
	);
	/* Fill colored empty space */
	for (
		col_i = left_len + num_input_len + coords_len;
		col_i < ed.win_size.ws_col;
		col_i++
	) {
		buf_write(buf, " ", 1);
	}
	/* Write right parts and end color */
	buf_write(buf, num_input, num_input_len);
	buf_write(buf, coords, coords_len);
}
