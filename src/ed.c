#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "buf.h"
#include "cfg.h"
#include "ed.h"
#include "err_alloc.h"
#include "esc.h"
#include "math.h"
#include "mode.h"
#include "path.h"
#include "term.h"
#include "win.h"

enum {
	ED_INPUT_NUM_RESET = -1, /* Flag to reset number input */
	ED_MSG_ARR_LEN = 64, /* Capacity of message buffer */
};

/* Editor options. */
struct Ed {
	Buf *buf; /* Buffer for all drawn content. */
	Win *win; /* Info about terminal's view. This is what the user sees */
	Mode mode; /* Input mode */
	char msg[ED_MSG_ARR_LEN]; /* Message for the user */
	size_t num_input; /* Number input. 0 if not set */
	unsigned char quit_presses_rem; /* Greater than 1 if file is dirty */
};

/* Deletes the inputed number of lines from file. */
static void ed_del_line(Ed *);

/* Draws status on last row. */
static void ed_draw_stat(Ed *, Buf *);

/* Drows left part of status. */
static size_t ed_draw_stat_left(Ed *, Buf *);

/* Draws right part of status. */
static void ed_draw_stat_right(const Ed *, Buf *, size_t);

/* Writes digit to the number input. Resets if argument is reset flag. */
static void ed_input_num(Ed *, char);

/* Inserts character to editor. */
static void ed_ins(Ed *, char);

/* Inserts below several empty lines. */
static void ed_ins_empty_line_below(Ed *);

/* Inserts on top several empty lines. */
static void ed_ins_empty_line_on_top(Ed *);

/* Use it when user presses quit key. Interacts with the remaining counter. */
static void ed_on_quit_press(Ed *);

/* Processes arrow key. Key must be between 'A' and 'D'. */
static void ed_proc_arrow_key(Ed *, char);

/* Process key in insertion mode. */
static void ed_proc_ins_key(Ed *, char);

/* Processes key sequence. Useful if single key press is several characters */
static void ed_proc_seq_key(Ed *, const char *, size_t);

/* Process key in normal mode. */
static void ed_proc_norm_key(Ed *, char);

/* Moves editor down. */
static void ed_mv_down(Ed *);

/* Moves editor left. */
static void ed_mv_left(Ed *);

/* Moves editor right. */
static void ed_mv_right(Ed *);

/* Moves editor up. */
static void ed_mv_up(Ed *);

/* Moves editor to begin of file. */
static void ed_mv_to_begin_of_file(Ed *);

/* Moves editor to begin of line. */
static void ed_mv_to_begin_of_line(Ed *);

/* Moves editor to end of line. */
static void ed_mv_to_end_of_file(Ed *);

/* Moves editor to end of line. */
static void ed_mv_to_end_of_line(Ed *);

/* Moves editor to next word. */
static void ed_mv_to_next_word(Ed *);

/* Moves editor to previous word. */
static void ed_mv_to_prev_word(Ed *);

/* Determines how many times the next action needs to be repeated. */
static size_t ed_repeat_times(const Ed *);

/* Saves opened file. */
static void ed_save_file(Ed *);

/* Saves opened file to spare dir. Useful if no privileges. */
static void ed_save_file_to_spare_dir(Ed *);

/* Sets formatted message to the user. */
static void ed_set_msg(Ed *, const char *, ...);

static void
ed_del_line(Ed *const ed)
{
	/* Try to delete lines or set error message */
	if (win_del_line(ed->win, ed_repeat_times(ed)) < 0)
		ed_set_msg(ed, "A single line in a file cannot be deleted.");
	else
		/* Set quit presses count after file change */
		ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
}

void
ed_draw(Ed *const ed)
{
	/* Go to start of window and clear the window */
	esc_go_home(ed->buf);
	esc_clr_win(ed->buf);

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
ed_draw_stat(Ed *const ed, Buf *const buf)
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
ed_draw_stat_left(Ed *const ed, Buf *const buf)
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
ed_draw_stat_right(const Ed *const ed, Buf *const buf, const size_t left_len)
{
	size_t col;
	char num_input[32] = {0};
	size_t right_len = 0;
	char pos[32] = {0};
	struct winsize winsize = win_size(ed->win);

	/* Write number input */
	if (ed->num_input > 0)
		right_len += snprintf(num_input, sizeof(num_input), "%zu < ", ed->num_input);

	/* Write current position in the file */
	right_len += snprintf(
		pos,
		sizeof(pos),
		"%zu, %zu ",
		win_curr_line_idx(ed->win),
		win_curr_line_cont_idx(ed->win)
	);

	/* Write empty space */
	for (col = left_len + right_len; col < winsize.ws_col; col++)
		buf_write(buf, " ", 1);

	/* Write right part */
	buf_writef(buf, "%s%s", num_input, pos);
}

void
ed_handle_signal(Ed *const ed, const int signal)
{
	/* Handle signals for window */
	win_handle_signal(ed->win, signal);

	/* Handle window size change signal */
	if (SIGWINCH == signal)
		/* Redraw after window resizing */
		ed_draw(ed);
}

static void
ed_input_num(Ed *const ed, const char digit)
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
ed_ins(Ed *const ed, const char ch)
{
	if (isprint(ch) || '\t' == ch) {
		win_ins(ed->win, ch);
		/* Set quit presses count after file change */
		ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	}
}

static void
ed_ins_empty_line_below(Ed *const ed)
{
	win_ins_empty_line_below(ed->win, ed_repeat_times(ed));

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	/* Switch mode for comfort */
	ed->mode = MODE_INS;
}

static void
ed_ins_empty_line_on_top(Ed *const ed)
{
	win_ins_empty_line_on_top(ed->win, ed_repeat_times(ed));

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	/* Switch mode for comfort */
	ed->mode = MODE_INS;
}

void
ed_mv_down(Ed *const ed)
{
	win_mv_down(ed->win, ed_repeat_times(ed));
}

void
ed_mv_left(Ed *const ed)
{
	win_mv_left(ed->win, ed_repeat_times(ed));
}

void
ed_mv_right(Ed *const ed)
{
	win_mv_right(ed->win, ed_repeat_times(ed));
}

void
ed_mv_up(Ed *const ed)
{
	win_mv_up(ed->win, ed_repeat_times(ed));
}

void
ed_mv_to_begin_of_file(Ed *const ed)
{
	win_mv_to_begin_of_file(ed->win);
}

void
ed_mv_to_begin_of_line(Ed *const ed)
{
	win_mv_to_begin_of_line(ed->win);
}

void
ed_mv_to_end_of_file(Ed *const ed)
{
	win_mv_to_end_of_file(ed->win);
}

void
ed_mv_to_end_of_line(Ed *const ed)
{
	win_mv_to_end_of_line(ed->win);
}

void
ed_mv_to_next_word(Ed *const ed)
{
	win_mv_to_next_word(ed->win, ed_repeat_times(ed));
}

void
ed_mv_to_prev_word(Ed *const ed)
{
	win_mv_to_prev_word(ed->win, ed_repeat_times(ed));
}

char
ed_need_to_quit(const Ed *const ed)
{
	return 0 == ed->quit_presses_rem;
}

static void
ed_on_quit_press(Ed *const ed)
{
	if (ed->quit_presses_rem > 0) {
		/* Decrease remaining quit presses */
		ed->quit_presses_rem--;

		/* Set message with remaining count if no need to quit */
		if (!ed_need_to_quit(ed))
			ed_set_msg(ed, "File is dirty. Presses remain: %hhu", ed->quit_presses_rem);
	}
}

Ed*
ed_open(const char *const path, const int ifd, const int ofd)
{
	/* Allocate opaque struct */
	Ed *const ed = err_malloc(sizeof(*ed));
	/* Allocate buffer for all drawn content */
	ed->buf = buf_alloc();
	/* Open window with accepted file and descriptors */
	ed->win = win_open(path, ifd, ofd);
	/* Set default editting mode */
	ed->mode = MODE_NORM;
	/* Set zero length to message */
	ed->msg[0] = 0;
	/* Make number input inactive */
	ed->num_input = 0;
	/* File is not dirty by default so we may quit using one key press */
	ed->quit_presses_rem = 1;
	return ed;
}

static void
ed_proc_arrow_key(Ed *const ed, const char key)
{
	switch (key) {
	case 'A':
		ed_mv_up(ed);
		break;
	case 'B':
		ed_mv_down(ed);
		break;
	case 'C':
		ed_mv_right(ed);
		break;
	case 'D':
		ed_mv_left(ed);
		break;
	}
}

static void
ed_proc_ins_key(Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_MODE_NORM:
		ed->mode = MODE_NORM;
		break;
	default:
		ed_ins(ed, key);
		break;
	}
}

static void
ed_proc_seq_key(Ed *const ed, const char *const seq, const size_t len)
{
	/* Arrows */
	if (
		3 == len
		&& '\x1b' == seq[0]
		&& '[' == seq[1]
		&& 'A' <= seq[2]
		&& seq[2] <= 'D'
	)
		ed_proc_arrow_key(ed, seq[2]);
}

static void
ed_proc_norm_key(Ed *const ed, const char key)
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
	case CFG_KEY_MODE_INS:
		ed->mode = MODE_INS;
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
	}
}

void
ed_quit(Ed *const ed)
{
	/* Free content buffer */
	buf_free(ed->buf);
	/* Close the window */
	win_close(ed->win);
	/* Free opaque struct */
	free(ed);
}

static size_t
ed_repeat_times(const Ed *const ed)
{
	return 0 == ed->num_input ? 1 : ed->num_input;
}

static void
ed_save_file(Ed *const ed)
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
ed_save_file_to_spare_dir(Ed *const ed)
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
ed_set_msg(Ed *const ed, const char *const fmt, ...)
{
	/* Collect arguments and print formatted message to field */
	va_list args;
	va_start(args, fmt);
	vsnprintf(ed->msg, sizeof(ed->msg), fmt, args);
	va_end(args);
}

void
ed_wait_and_proc_key(Ed *const ed)
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
		}
	}

	/* Process number input */
	if ('0' <= seq[0] && seq[0] <= '9') {
		ed_input_num(ed, seq[0] - '0');
	} else {
		ed_input_num(ed, ED_INPUT_NUM_RESET);
	}
}
