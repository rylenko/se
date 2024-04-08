#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "alloc.h"
#include "buf.h"
#include "cfg.h"
#include "ed.h"
#include "esc.h"
#include "math.h"
#include "mode.h"
#include "path.h"
#include "term.h"
#include "win.h"

enum {
	/* Search */
	ED_SEARCH_INPUT_ARR_LEN = 64, /* Capacity of search query buffer */
	ED_INPUT_SEARCH_CLEAR = -2, /* Flag to clear search input */
	ED_INPUT_SEARCH_DEL_CHAR = -1, /* Flag to delete last character in search */
	/* Other */
	ED_INPUT_NUM_RESET = -1, /* Flag to reset number input */
	ED_MSG_ARR_LEN = 64, /* Capacity of message buffer */
	ED_STAT_RIGHT_ARR_LEN = 128, /* Capacity of right part of the status */
};

/* Editor options. */
struct Ed {
	Buf *buf; /* Buffer for all drawn content. */
	Win *win; /* Info about terminal's view. This is what the user sees */
	enum Mode mode; /* Input mode */
	char msg[ED_MSG_ARR_LEN]; /* Message for the user */
	size_t num_input; /* Number input. 0 if not set */
	char search_input[ED_SEARCH_INPUT_ARR_LEN]; /* Search input */
	size_t search_input_len; /* Search query input length */
	unsigned char quit_presses_rem; /* Greater than 1 if file is dirty */
	volatile sig_atomic_t sigwinch; /* Resize flag. See signal-safety(7) */
};

/* Breaks current line at the current cursor's position */
static void ed_break_line(struct Ed *);

/* Deletes character before the cursor. */
static void ed_del_char(struct Ed *);

/* Deletes the inputed number of lines from file. */
static void ed_del_line(struct Ed *);

/* Draws status on last row. */
static void ed_draw_stat(struct Ed *, Buf *);

/* Drows left part of status. */
static size_t ed_draw_stat_left(struct Ed *, Buf *);

/* Draws right part of status. */
static void ed_draw_stat_right(const struct Ed *, Buf *, size_t);

/* Writes digit to the number input. Resets if argument is reset flag. */
static void ed_input_num(struct Ed *, char);

/*
Writes character to the search input.

Deletes last character if argument is delete flag and resets if argument is
reset flag.
*/
static void ed_input_search(struct Ed *, char);

/* Inserts character to editor. */
static void ed_ins_char(struct Ed *, char);

/* Inserts below several empty lines. */
static void ed_ins_empty_line_below(struct Ed *);

/* Inserts on top several empty lines. */
static void ed_ins_empty_line_on_top(struct Ed *);

/* Use it when user presses quit key. Interacts with the remaining counter. */
static void ed_on_quit_press(struct Ed *);

/* Processes arrow key. Key must be between 'A' and 'D'. */
static void ed_proc_arrow_key(struct Ed *, enum ArrowKey);

/* Process key in insertion mode. */
static void ed_proc_ins_key(struct Ed *, char);

/* Process key in search mode. */
static void ed_proc_search_key(struct Ed *, char);

/* Processes key sequence. Useful if single key press is several characters */
static void ed_proc_seq_key(struct Ed *, const char *, size_t);

/* Process key in normal mode. */
static void ed_proc_norm_key(struct Ed *, char);

/* Moves editor down. */
static void ed_mv_down(struct Ed *);

/* Moves editor left. */
static void ed_mv_left(struct Ed *);

/* Moves editor right. */
static void ed_mv_right(struct Ed *);

/* Moves editor up. */
static void ed_mv_up(struct Ed *);

/* Moves editor to begin of file. */
static void ed_mv_to_begin_of_file(struct Ed *);

/* Moves editor to begin of line. */
static void ed_mv_to_begin_of_line(struct Ed *);

/* Moves editor to end of line. */
static void ed_mv_to_end_of_file(struct Ed *);

/* Moves editor to end of line. */
static void ed_mv_to_end_of_line(struct Ed *);

/* Moves editor to next word. */
static void ed_mv_to_next_word(struct Ed *);

/* Moves editor to previous word. */
static void ed_mv_to_prev_word(struct Ed *);

/* Determines how many times the next action needs to be repeated. */
static size_t ed_repeat_times(const struct Ed *);

/* Saves opened file. */
static void ed_save_file(struct Ed *);

/* Saves opened file to spare dir. Useful if no privileges. */
static void ed_save_file_to_spare_dir(struct Ed *);

/* Searches backward with inputed search query. */
static void ed_search_bwd(struct Ed *const ed);

/* Searches forward with inputed search query. */
static void ed_search_fwd(struct Ed *const ed);

/* Sets formatted message to the user. */
static void ed_set_msg(struct Ed *, const char *, ...);

/* Switches editor to passed mode. */
static void ed_switch_mode(struct Ed *, enum Mode);

static void
ed_break_line(struct Ed *const ed)
{
	win_break_line(ed->win);
	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
}

static void
ed_del_char(struct Ed *const ed)
{
	win_del_char(ed->win);
	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
}

static void
ed_del_line(struct Ed *const ed)
{
	/* Try to delete lines or set error message */
	if (win_del_line(ed->win, ed_repeat_times(ed)) == -1)
		ed_set_msg(ed, "A single line in a file cannot be deleted.");
	else
		/* Set quit presses count after file change */
		ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
}

void
ed_draw(struct Ed *const ed)
{
	/* Go to start of window and clear the window */
	esc_go_home(ed->buf);
	esc_clr_win(ed->buf);

	/* Check flag to update window size. See signal-safety(7) for more */
	if (ed->sigwinch) {
		win_upd_size(ed->win);
		ed->sigwinch = 0;
	}

	if (!ed_need_to_quit(ed)) {
		/* Hide cursor to not flicker */
		esc_cur_hide(ed->buf);

		/* Draw lines of file */
		win_draw_lines(ed->win, ed->buf);
		/* Draw status */
		ed_draw_stat(ed, ed->buf);
		/* Draw expanded cursor */
		win_draw_cur(ed->win, ed->buf);

		/* Show hidden cursor */
		esc_cur_show(ed->buf);
	}

	/* Flush the buffer to terminal */
	term_flush(ed->buf);
}

static void
ed_draw_stat(struct Ed *const ed, Buf *const buf)
{
	size_t left_len = 0;

	/* Begin colored output */
	esc_color_begin(buf, &cfg_color_stat_fg, &cfg_color_stat_bg);

	/* Draw left and right parts of status */
	left_len += ed_draw_stat_left(ed, buf);
	ed_draw_stat_right(ed, buf, left_len);

	/* End colored output */
	esc_color_end(buf);
}

static size_t
ed_draw_stat_left(struct Ed *const ed, Buf *const buf)
{
	/* Get filename of opened file */
	const char *const filename = path_get_filename(win_file_path(ed->win));
	size_t len = 0;

	/* Write mode and opened file's name */
	len += buf_writef(buf, " %s > %s", mode_str(ed->mode), filename);

	/* Add mark if file is dirty */
	if (win_file_is_dirty(ed->win))
		len += buf_write(buf, " [+]", 4);

	/* Write message if exists */
	if (ed->msg[0] != 0) {
		len += buf_writef(buf, ": %s", ed->msg);
		ed->msg[0] = 0;
	}
	return len;
}

static void
ed_draw_stat_right(
	const struct Ed *const ed,
	Buf *const buf,
	const size_t left_len
) {
	size_t col;
	const size_t cont_idx = win_curr_line_cont_idx(ed->win);
	const size_t idx = win_curr_line_idx(ed->win);
	char right[ED_STAT_RIGHT_ARR_LEN];
	size_t right_len = 0;
	const struct winsize winsize = win_size(ed->win);

	/* Write right part of the status */
	switch (ed->mode) {
	case MODE_NORM:
		right_len += snprintf(
			right,
			sizeof(right),
			"%zu < %zu, %zu ",
			ed->num_input,
			idx,
			cont_idx
		);
		break;
	case MODE_SEARCH:
		right_len += snprintf(
			right,
			sizeof(right),
			"%s < %zu, %zu ",
			ed->search_input,
			idx,
			cont_idx
		);
		break;
	default:
		right_len += snprintf(right, sizeof(right), "%zu, %zu ", idx, cont_idx);
		break;
	}

	/* Write empty space */
	for (col = left_len + right_len; col < winsize.ws_col; col++)
		buf_write(buf, " ", 1);

	/* Write right part */
	buf_writef(buf, right, right_len);
}

void
ed_handle_signal(struct Ed *const ed, const int signal)
{
	/*
	Set sigwinch flag to resize later in not async-signal-safe function. See
	signal-safety(7) for more
	*/
	if (SIGWINCH == signal)
		ed->sigwinch = 1;
}

static void
ed_input_num(struct Ed *const ed, const char digit)
{
	/* Validate digit */
	assert((0 <= digit && digit <= 9) || ED_INPUT_NUM_RESET == digit);

	/* Zeroize input if current digit overflows or need to reset */
	if (ED_INPUT_NUM_RESET == digit || (SIZE_MAX - digit) / 10 < ed->num_input)
		ed->num_input = 0;
	else
		ed->num_input = (ed->num_input * 10) + digit;
}

static void
ed_input_search(struct Ed *const ed, const char ch)
{
	if (ED_INPUT_SEARCH_CLEAR == ch) {
		/* Reset search input */
		ed->search_input_len = 0;
		ed->search_input[0] = 0;
	} else if (ED_INPUT_SEARCH_DEL_CHAR == ch && ed->search_input_len > 0) {
		/* Delete last character in the input */
		ed->search_input[--ed->search_input_len] = 0;
	} else if (ed->search_input_len + 1 < sizeof(ed->search_input) && isprint(ch))
	{
		/* Write new character if there is place for null byte */
		ed->search_input[ed->search_input_len++] = ch;
		ed->search_input[ed->search_input_len] = 0;
	}
}

static void
ed_ins_char(struct Ed *const ed, const char ch)
{
	if (isprint(ch) || '\t' == ch) {
		win_ins_char(ed->win, ch);
		/* Set quit presses count after file change */
		ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	}
}

static void
ed_ins_empty_line_below(struct Ed *const ed)
{
	win_ins_empty_line_below(ed->win, ed_repeat_times(ed));

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	/* Switch mode for comfort */
	ed->mode = MODE_INS;
}

static void
ed_ins_empty_line_on_top(struct Ed *const ed)
{
	win_ins_empty_line_on_top(ed->win, ed_repeat_times(ed));

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	/* Switch mode for comfort */
	ed->mode = MODE_INS;
}

static void
ed_mv_down(struct Ed *const ed)
{
	win_mv_down(ed->win, ed_repeat_times(ed));
}

static void
ed_mv_left(struct Ed *const ed)
{
	win_mv_left(ed->win, ed_repeat_times(ed));
}

static void
ed_mv_right(struct Ed *const ed)
{
	win_mv_right(ed->win, ed_repeat_times(ed));
}

static void
ed_mv_up(struct Ed *const ed)
{
	win_mv_up(ed->win, ed_repeat_times(ed));
}

static void
ed_mv_to_begin_of_file(struct Ed *const ed)
{
	win_mv_to_begin_of_file(ed->win);
}

static void
ed_mv_to_begin_of_line(struct Ed *const ed)
{
	win_mv_to_begin_of_line(ed->win);
}

static void
ed_mv_to_end_of_file(struct Ed *const ed)
{
	win_mv_to_end_of_file(ed->win);
}

static void
ed_mv_to_end_of_line(struct Ed *const ed)
{
	win_mv_to_end_of_line(ed->win);
}

static void
ed_mv_to_next_word(struct Ed *const ed)
{
	win_mv_to_next_word(ed->win, ed_repeat_times(ed));
}

static void
ed_mv_to_prev_word(struct Ed *const ed)
{
	win_mv_to_prev_word(ed->win, ed_repeat_times(ed));
}

char
ed_need_to_quit(const struct Ed *const ed)
{
	return 0 == ed->quit_presses_rem;
}

static void
ed_on_quit_press(struct Ed *const ed)
{
	if (ed->quit_presses_rem > 0) {
		/* Decrease remaining quit presses */
		ed->quit_presses_rem--;

		/* Set message with remaining count if no need to quit */
		if (!ed_need_to_quit(ed))
			ed_set_msg(ed, "File is dirty. Presses remain: %hhu", ed->quit_presses_rem);
	}
}

struct Ed*
ed_open(const char *const path, const int ifd, const int ofd)
{
	/* Allocate opaque struct */
	struct Ed *const ed = malloc_err(sizeof(*ed));
	/* Allocate buffer for all drawn content */
	ed->buf = buf_alloc();
	/* Open window with accepted file and descriptors */
	ed->win = win_open(path, ifd, ofd);
	/* Set default editting mode */
	ed_switch_mode(ed, MODE_NORM);
	/* Set zero length to message */
	ed->msg[0] = 0;
	/* Make number input inactive */
	ed_input_num(ed, ED_INPUT_NUM_RESET);
	/* Set zero length to search query */
	ed_input_search(ed, ED_INPUT_SEARCH_CLEAR);
	/* File is not dirty by default so we may quit using one key press */
	ed->quit_presses_rem = 1;
	/* Set signal default values */
	ed->sigwinch = 0;
	return ed;
}

static void
ed_proc_arrow_key(struct Ed *const ed, const enum ArrowKey key)
{
	switch (key) {
	case ARROW_KEY_UP:
		ed_mv_up(ed);
		break;
	case ARROW_KEY_DOWN:
		ed_mv_down(ed);
		break;
	case ARROW_KEY_RIGHT:
		ed_mv_right(ed);
		break;
	case ARROW_KEY_LEFT:
		ed_mv_left(ed);
		break;
	}
}

static void
ed_proc_ins_key(struct Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_DEL_CHAR:
		ed_del_char(ed);
		break;
	case CFG_KEY_INS_LINE_BREAK:
		ed_break_line(ed);
		break;
	case CFG_KEY_MODE_INS_TO_NORM:
		ed->mode = MODE_NORM;
		break;
	default:
		ed_ins_char(ed, key);
		break;
	}
}

static void
ed_proc_norm_key(struct Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_DEL_LINE:
		ed_del_line(ed);
		break;
	case CFG_KEY_INS_LINE_BELOW:
		ed_ins_empty_line_below(ed);
		return;
	case CFG_KEY_INS_LINE_ON_TOP:
		ed_ins_empty_line_on_top(ed);
		return;
	case CFG_KEY_MODE_NORM_TO_INS:
		ed->mode = MODE_INS;
		break;
	case CFG_KEY_MODE_NORM_TO_SEARCH:
		ed_switch_mode(ed, MODE_SEARCH);
		break;
	case CFG_KEY_QUIT:
		ed_on_quit_press(ed);
		break;
	case CFG_KEY_SAVE:
		ed_save_file(ed);
		break;
	case CFG_KEY_SAVE_TO_SPARE_DIR:
		ed_save_file_to_spare_dir(ed);
		break;
	case CFG_KEY_MV_DOWN:
		ed_mv_down(ed);
		break;
	case CFG_KEY_MV_LEFT:
		ed_mv_left(ed);
		break;
	case CFG_KEY_MV_RIGHT:
		ed_mv_right(ed);
		break;
	case CFG_KEY_MV_UP:
		ed_mv_up(ed);
		break;
	case CFG_KEY_MV_TO_BEGIN_OF_FILE:
		ed_mv_to_begin_of_file(ed);
		break;
	case CFG_KEY_MV_TO_BEGIN_OF_LINE:
		ed_mv_to_begin_of_line(ed);
		break;
	case CFG_KEY_MV_TO_END_OF_FILE:
		ed_mv_to_end_of_file(ed);
		break;
	case CFG_KEY_MV_TO_END_OF_LINE:
		ed_mv_to_end_of_line(ed);
		break;
	case CFG_KEY_MV_TO_NEXT_WORD:
		ed_mv_to_next_word(ed);
		break;
	case CFG_KEY_MV_TO_PREV_WORD:
		ed_mv_to_prev_word(ed);
		break;
	case CFG_KEY_SEARCH_BWD:
		ed_search_bwd(ed);
		break;
	case CFG_KEY_SEARCH_FWD:
		ed_search_fwd(ed);
		break;
	}

	/* Process number input */
	if ('0' <= key && key <= '9')
		ed_input_num(ed, key - '0');
	else
		ed_input_num(ed, ED_INPUT_NUM_RESET);
}

static void
ed_proc_search_key(struct Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_MODE_SEARCH_TO_NORM:
		ed_search_fwd(ed);
		/* FALLTHROUGH */
	case CFG_KEY_MODE_SEARCH_TO_NORM_CANCEL:
		ed_switch_mode(ed, MODE_NORM);
		break;
	case CFG_KEY_SEARCH_DEL_CHAR:
		ed_input_search(ed, ED_INPUT_SEARCH_DEL_CHAR);
		break;
	default:
		ed_input_search(ed, key);
		break;
	}
}

static void
ed_proc_seq_key(struct Ed *const ed, const char *const seq, const size_t len)
{
	enum ArrowKey arrow_key;
	/* Arrows */
	if (esc_get_arrow_key(seq, len, &arrow_key) != -1)
		ed_proc_arrow_key(ed, arrow_key);
}

void
ed_quit(struct Ed *const ed)
{
	/* Free content buffer */
	buf_free(ed->buf);
	/* Close the window */
	win_close(ed->win);
	/* Free opaque struct */
	free(ed);
}

static size_t
ed_repeat_times(const struct Ed *const ed)
{
	return 0 == ed->num_input ? 1 : ed->num_input;
}

static void
ed_save_file(struct Ed *const ed)
{
	/* Save file to current file path */
	size_t len = win_save_file(ed->win);

	/* Check save failed */
	if (0 == len) {
		ed_set_msg(ed, "Failed to save: %s", strerror(errno));
	} else {
		ed_set_msg(ed, "%zu bytes saved", len);
		/* Update quit presses */
		ed->quit_presses_rem = 1;
	}
}

static void
ed_save_file_to_spare_dir(struct Ed *const ed)
{
	char path[CFG_SPARE_PATH_MAX_LEN + 1];
	/* Save file to the spare dir */
	size_t len = win_save_file_to_spare_dir(ed->win, path, sizeof(path));
	/* Set message */
	ed_set_msg(ed, "%zu bytes saved to %s", len, path);
	/* Update quit presses */
	ed->quit_presses_rem = 1;
}

static void
ed_search_bwd(struct Ed *const ed)
{
	win_search_bwd(ed->win, ed->search_input);
}

static void
ed_search_fwd(struct Ed *const ed)
{
	win_search_fwd(ed->win, ed->search_input);
}

static void
ed_set_msg(struct Ed *const ed, const char *const fmt, ...)
{
	/* Collect arguments and print formatted message to field */
	va_list args;
	va_start(args, fmt);
	vsnprintf(ed->msg, sizeof(ed->msg), fmt, args);
	va_end(args);
}

static void
ed_switch_mode(struct Ed *const ed, const enum Mode mode)
{
	switch (mode) {
	case MODE_SEARCH:
		ed_input_search(ed, ED_INPUT_SEARCH_CLEAR);
		/* FALLTHROUGH */
	default:
		ed->mode = mode;
	}
}

void
ed_wait_and_proc_key(struct Ed *const ed)
{
	char seq[3];
	/* Wait key press */
	size_t seq_len = term_wait_key(seq, sizeof(seq));

	/* Process key if key is more than one character */
	if (seq_len > 1) {
		ed_proc_seq_key(ed, seq, seq_len);
	} else {
		switch (ed->mode) {
		case MODE_NORM:
			/* Process key in normal mode */
			ed_proc_norm_key(ed, seq[0]);
			break;
		case MODE_INS:
			/* Process key in insertion mode */
			ed_proc_ins_key(ed, seq[0]);
			break;
		case MODE_SEARCH:
			/* Process key in search mode */
			ed_proc_search_key(ed, seq[0]);
			break;
		}
	}
}
