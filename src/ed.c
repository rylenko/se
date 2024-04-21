#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "cfg.h"
#include "ed.h"
#include "esc.h"
#include "file.h"
#include "math.h"
#include "mode.h"
#include "path.h"
#include "term.h"
#include "vec.h"
#include "win.h"

enum {
	ED_INPUT_SEARCH_CLEAR = -2, /* Flag to clear search input */
	ED_INPUT_SEARCH_DEL_CHAR = -1, /* Flag to delete last character in search */
	ED_INPUT_NUM_RESET = -1, /* Flag to reset number input */
};

/* Editor options. */
struct Ed {
	Vec *buf; /* Buffer for all drawn content. */
	Win *win; /* Info about terminal's view. This is what the user sees */
	enum Mode mode; /* Input mode */
	char msg[64]; /* Message for the user */
	size_t num_input; /* Number input. 0 if not set */
	char search_input[64]; /* Search input */
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
static void ed_draw_stat(struct Ed *);

/* Flush editor's drawing buffer. */
static void ed_flush_buf(struct Ed *);

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

/* Processes arrow key. */
static void ed_proc_arrow_key(struct Ed *, const char *, size_t);

/* Process key in insertion mode. */
static void ed_proc_ins_key(struct Ed *, char);

/* Processes mouse wheel key. */
static void ed_proc_mouse_wh_key(struct Ed *, const char *, size_t);

/* Process key in normal mode. */
static void ed_proc_norm_key(struct Ed *, char);

/* Process key in search mode. */
static void ed_proc_search_key(struct Ed *, char);

/* Processes key sequence. Useful if single key press is several characters */
static void ed_proc_seq_key(struct Ed *, const char *, size_t);

/* Determines how many times the next action needs to be repeated. */
static size_t ed_repeat_times(const struct Ed *);

/* Saves opened file. */
static void ed_save_file(struct Ed *);

/* Saves opened file to spare dir. Useful if no privileges. */
static void ed_save_file_to_spare_dir(struct Ed *);

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

static int
ed_del_line(struct Ed *const ed)
{
	int ret;

	/* Try to delete lines or set error message */
	ret = win_del_line(ed->win, ed_repeat_times(ed));
	if (0 == ret) {
		/* Set quit presses count after file change */
		ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
		return 0;
	}
	/* Check if tried to delete last line */
	if (ENOSYS == errno) {
		ed_set_msg(ed, "A single line in a file cannot be deleted.");
		return 0;
	}
	return -1;
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

	/* Hide cursor to not flicker */
	esc_cur_hide(ed->buf);
	/* Draw lines of file */
	win_draw_lines(ed->win, ed->buf);
	/* Draw status */
	ed_draw_stat(ed);
	/* Draw expanded cursor */
	win_draw_cur(ed->win, ed->buf);
	/* Show hidden cursor */
	esc_cur_show(ed->buf);

	/* Flush the buffer to terminal */
	ed_flush_buf(ed);
}

static void
ed_draw_stat(struct Ed *const ed)
{
	size_t i;
	char s[128];
	size_t len;
	size_t left_len = 0;
	struct winsize winsize = win_size(ed->win);
	/* Get filename of opened file */
	const char *const fname = path_get_fname(win_file_path(ed->win));
	/* Get coordinates */
	const size_t y = win_curr_line_idx(ed->win);
	const size_t x = win_curr_line_char_idx(ed->win);

	/* Begin colored output */
	esc_color_begin(ed->buf, &cfg_color_stat_fg, &cfg_color_stat_bg);

	/* Draw mode and filename */
	len = snprintf(s, sizeof(s), " %s > %s", mode_str(ed->mode), fname);
	left_len += len;
	vec_append(ed->buf, s, len);

	/* Add mark if file is dirty */
	if (win_file_is_dirty(ed->win)) {
		len = snprintf(s, sizeof(s), " [+]");
		left_len += len;
		vec_append(ed->buf, s, len);
	}

	/* Draw message if set */
	if (ed->msg[0] != 0) {
		len = snprintf(s, sizeof(s), ": %s", ed->msg);
		left_len += len;
		vec_append(ed->buf, s, len);

		/* Reset message to do not draw on the next draw */
		ed->msg[0] = 0;
	}

	/* Prepare length and formatted string for the right part */
	switch (ed->mode) {
	case MODE_NORM:
		len = snprintf(s, sizeof(s), "%zu < %zu, %zu ", ed->num_input, y, x);
		break;
	case MODE_SEARCH:
		len = snprintf(s, sizeof(s), "%s < %zu, %zu ", ed->search_input, y, x);
		break;
	default:
		len = snprintf(s, sizeof(s), "%zu, %zu ", y, x);
		break;
	}
	/* Draw colored empty space */
	for (i = left_len + len; i < winsize.ws_col; i++)
		vec_append(ed->buf, " ", 1);
	/* Draw the right part */
	vec_append(ed->buf, s, MIN(len, winsize.ws_col - left_len));

	/* End colored output */
	esc_color_end(ed->buf);
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
ed_flush_buf(struct Ed *const ed)
{
	int ret;
	/* Write buffer to terminal */
	ssize_t len = term_write(vec_items(ed->buf), vec_len(ed->buf));
	if (-1 == len)
		return -1;

	/* Set the length to zero to continue appending characters to the beginning */
	ret = vec_set_len(ed->buf, 0);
	if (-1 == ret)
		return -1;
	return 0;
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
			ed_set_msg(ed, "Changes not saved. Presses: %hhu.", ed->quit_presses_rem);
	}
}

struct Ed*
ed_open(const char *const path, const int ifd, const int ofd)
{
	/* Allocate opaque struct */
	struct Ed *const ed = malloc_err(sizeof(*ed));
	/* Allocate buffer for all drawn content */
	ed->buf = vec_alloc(sizeof(char), 4096);
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

	/* Enable alternate screen. It will be set during first drawing */
	esc_alt_scr_on(ed->buf);
	/* Enable mouse wheel tracking. It will be set during first drawing */
	esc_mouse_wh_track_on(ed->buf);
	return ed;
}

static void
ed_proc_arrow_key(struct Ed *const ed, const char *const seq, const size_t len)
{
	int ret;
	enum ArrowKey key;
	const size_t repeat_times = ed_repeat_times(ed);
	int (*proc)(Win *, size_t);

	/* Try to extract arrow key */
	ret = esc_extr_arrow_key(seq, len, &arrow_key);
	if (-1 == ret) {
		/* Skip invalid value error because we just tried */
		if (EINVAL != errno)
			return -1;
		errno = 0;
		return 0;
	}
	/* Get processor */
	switch (key) {
	case ARROW_KEY_UP:
		proc = win_mv_up;
		break;
	case ARROW_KEY_DOWN:
		proc = win_mv_down;
		break;
	case ARROW_KEY_RIGHT:
		proc = win_mv_right;
		break;
	case ARROW_KEY_LEFT:
		proc = win_mv_left;
		break;
	}
	/* Process */
	ret = proc(ed->win, repeat_times);
	if (-1 == ret)
		return -1;
	return 0;
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
ed_proc_mouse_wh_key(
	struct Ed *const ed,
	const char *const seq,
	const size_t len
) {
	int ret;
	enum MouseWhKey;
	const size_t repeat_times = ed_repeat_times(ed);
	int (*proc)(Win *, size_t);

	/* Try to extract mouse wheel key */
	ret = esc_extr_mouse_wh_key(seq, len, &mouse_wh_key);
	if (-1 == ret) {
		/* Skip invalid value error because we just tried */
		if (EINVAL != errno)
			return -1;
		errno = 0;
		return 0;
	}
	/* Get processor */
	switch (key) {
	case MOUSE_WH_KEY_UP:
		proc = win_mv_up;
		break;
	case MOUSE_WH_KEY_DOWN:
		proc = win_mv_down;
		break;
	}
	/* Process */
	ret = proc(ed->win, repeat_times);
	if (-1 == ret)
		return -1;
	return 0;
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
		win_mv_down(ed->win, ed_repeat_times(ed));
		break;
	case CFG_KEY_MV_LEFT:
		win_mv_left(ed->win, ed_repeat_times(ed));
		break;
	case CFG_KEY_MV_RIGHT:
		win_mv_right(ed->win, ed_repeat_times(ed));
		break;
	case CFG_KEY_MV_UP:
		win_mv_up(ed->win, ed_repeat_times(ed));
		break;
	case CFG_KEY_MV_TO_BEGIN_OF_FILE:
		win_mv_to_begin_of_file(ed->win);
		break;
	case CFG_KEY_MV_TO_BEGIN_OF_LINE:
		win_mv_to_begin_of_line(ed->win);
		break;
	case CFG_KEY_MV_TO_END_OF_FILE:
		win_mv_to_end_of_file(ed->win);
		break;
	case CFG_KEY_MV_TO_END_OF_LINE:
		win_mv_to_end_of_line(ed->win);
		break;
	case CFG_KEY_MV_TO_NEXT_WORD:
		win_mv_to_next_word(ed->win, ed_repeat_times(ed));
		break;
	case CFG_KEY_MV_TO_PREV_WORD:
		win_mv_to_prev_word(ed->win, ed_repeat_times(ed));
		break;
	case CFG_KEY_SEARCH_BWD:
		win_search(ed->win, ed->search_input, DIR_BWD);
		break;
	case CFG_KEY_SEARCH_FWD:
		win_search(ed->win, ed->search_input, DIR_FWD);
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
		win_search(ed->win, ed->search_input, DIR_FWD);
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

static int
ed_proc_seq_key(struct Ed *const ed, const char *const seq, const size_t len)
{
	int ret;
	enum ArrowKey arrow_key;
	enum MouseWhKey mouse_wh_key;

	/* Try to process arrow key */
	ret = ed_proc_arrow_key(ed, seq, len);
	if (-1 == ret)
		return -1;

	/* Try to process mouse wheel key */
	ret = ed_proc_mouse_wh_key(ed, seq, len);
	if (-1 == ret)
		return -1;
	return 0;
}

void
ed_quit(struct Ed *const ed)
{
	/* Disable alternate screen */
	esc_alt_scr_off(ed->buf);
	/* Disable mouse wheel tracking */
	esc_mouse_wh_track_off(ed->buf);
	/* Flush settings disabling */
	ed_flush_buf(ed);

	/* Free content buffer */
	vec_free(ed->buf);
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
		ed_set_msg(ed, "Failed to save: %s.", strerror(errno));
	} else {
		ed_set_msg(ed, "%zu bytes saved.", len);
		/* Update quit presses */
		ed->quit_presses_rem = 1;
	}
}

static int
ed_save_file_to_spare_dir(struct Ed *const ed)
{
	char path[CFG_SPARE_PATH_MAX_LEN + 1];

	/* Save file to the spare dir */
	size_t len = win_save_file_to_spare_dir(ed->win, path, sizeof(path));
	if (0 == len) {
		/* Assert that path buffer has enough size */
		assert(ENOBUFS != errno);
		return -1;
	}

	/* Set message */
	ed_set_msg(ed, "%zu bytes saved to %s.", len, path);

	/* Update quit presses */
	ed->quit_presses_rem = 1;
	return 0;
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
	char seq[4];
	/* Wait key press */
	size_t seq_len = term_wait_key(seq, sizeof(seq));

	if (seq_len > 1) {
		/* Process key sequence if more than one characters readed */
		ed_proc_seq_key(ed, seq, seq_len);
	} else {
		/* Process single character keys in different input modes */
		switch (ed->mode) {
		case MODE_NORM:
			ed_proc_norm_key(ed, seq[0]);
			break;
		case MODE_INS:
			ed_proc_ins_key(ed, seq[0]);
			break;
		case MODE_SEARCH:
			ed_proc_search_key(ed, seq[0]);
			break;
		}
	}
}
