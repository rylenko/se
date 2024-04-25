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

/*
Breaks current line at the current cursor's position.

Returns 0 on success and -1 on error.
*/
static int ed_break_line(struct Ed *);

/*
Deletes character before the cursor.

Returns 0 on success and -1 on error.
*/
static int ed_del_char(struct Ed *);

/*
Deletes the inputed number of lines from file.

Returns 0 on success and -1 on error.
*/
static int ed_del_line(struct Ed *);

/*
Draws status on last row.

Returns 0 on success and -1 on error.
*/
static int ed_draw_stat(struct Ed *);

/*
Flush editor's drawing buffer.

Returns 0 on success and -1 on error.
*/
static int ed_flush_buf(struct Ed *);

/*
Inserts character to editor.

Returns 0 on success and -1 on error.
*/
static int ed_ins_char(struct Ed *, char);

/*
Inserts below several empty lines.

Returns 0 on success and -1 on error.
*/
static int ed_ins_empty_line_below(struct Ed *);

/*
Inserts on top several empty lines.

Returns 0 on success and -1 on error.
*/
static int ed_ins_empty_line_on_top(struct Ed *);

/*
Writes digit to the number input. Clears if overflows.

Returns 0 on success and -1 on error.

Sets `EINVAL` if argument is invalid.
*/
static int ed_num_input(struct Ed *, char);

/* Clears number input. */
static void ed_num_input_clr(struct Ed *);

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

/*
Writes character to the search input.

Returns 0 on success and -1 on error.

Sets `EINVAL` if character is invalid.
*/
static void ed_search_input(struct Ed *, char);

/* Clears search input. */
static void ed_search_input_clr(struct Ed *);

/* Deletes last character from the input if exists. */
static void ed_search_input_del_ch(struct Ed *);

/* Sets formatted message to the user. */
static void ed_set_msg(struct Ed *, const char *, ...);

/* Switches editor to passed mode. */
static void ed_switch_mode(struct Ed *, enum Mode);

static int
ed_break_line(struct Ed *const ed)
{
	/* Break line */
	int ret = win_break_line(ed->win);
	if (-1 == ret)
		return -1;

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	return 0;
}

static int
ed_del_char(struct Ed *const ed)
{
	/* Delete character */
	int ret = win_del_char(ed->win);
	if (-1 == ret)
		return -1;

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	return 0;
}

static int
ed_del_line(struct Ed *const ed)
{
	/* Try to delete lines or set error message */
	int ret = win_del_line(ed->win, ed_repeat_times(ed));
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

int
ed_draw(struct Ed *const ed)
{
	/* Go to start of window and clear the window */
	int ret = esc_go_home(ed->buf);
	if (-1 == ret)
		return -1;
	ret = esc_clr_win(ed->buf);
	if (-1 == ret)
		return -1;

	/* Check flag to update window size. See signal-safety(7) for more */
	if (ed->sigwinch) {
		ed->sigwinch = 0;

		/* Update window size */
		ret = win_upd_size(ed->win);
		if (-1 == ret)
			return ret;
	}

	/* Hide cursor to not flicker */
	ret = esc_cur_hide(ed->buf);
	if (-1 == ret)
		return -1;
	/* Draw lines of file */
	ret = win_draw_lines(ed->win, ed->buf);
	if (-1 == ret)
		return -1;
	/* Draw status */
	ret = ed_draw_stat(ed);
	if (-1 == ret)
		return -1;
	/* Draw expanded cursor */
	ret = win_draw_cur(ed->win, ed->buf);
	if (-1 == ret)
		return -1;
	/* Show hidden cursor */
	ret = esc_cur_show(ed->buf);
	if (-1 == ret)
		return -1;
	/* Flush the buffer to terminal */
	ret = ed_flush_buf(ed);
	if (-1 == ret)
		return -1;
	return 0;
}

/* TODO: split into smaller functions. */
static int
ed_draw_stat(struct Ed *const ed)
{
	int ret;
	size_t i;
	size_t left_len = 0;
	struct winsize winsize = win_size(ed->win);
	const char *const fname = path_get_fname(win_file_path(ed->win));
	const size_t y = win_curr_line_idx(ed->win);
	const size_t x = win_curr_line_char_idx(ed->win);
	char r[128];

	/* Begin colored output */
	ret = esc_color_begin(ed->buf, &cfg_color_stat_fg, &cfg_color_stat_bg);
	if (-1 == ret)
		return -1;

	/* Draw mode and filename */
	ret = vec_append_fmt(ed->buf, " %s > %s", mode_str(ed->mode), fname);
	if (-1 == ret)
		return -1;
	left_len += ret;

	/* Add mark if file is dirty */
	if (win_file_is_dirty(ed->win)) {
		ret = vec_append(ed->buf, " [+]", 4);
		if (-1 == ret)
			return -1;
		left_len += 4;
	}

	/* Draw message if set */
	if (ed->msg[0] != 0) {
		/* Draw message */
		ret = vec_append(ed->buf, ": %s", ed->msg);
		if (-1 == ret)
			return -1;
		left_len += ret;

		/* Reset message to do not draw on the next draw */
		ed->msg[0] = 0;
	}

	/* Prepare length and formatted string for the right part */
	switch (ed->mode) {
	case MODE_NORM:
		r_len = snprintf(r, sizeof(r), "%zu < %zu, %zu ", ed->num_input, y, x);
		break;
	case MODE_SEARCH:
		r_len = snprintf(r, sizeof(r), "%s < %zu, %zu ", ed->search_input, y, x);
		break;
	default:
		r_len = snprintf(r, sizeof(r), "%zu, %zu ", y, x);
		break;
	}
	if (r_len < 0 || r_len >= sizeof(r))
		return -1;

	/* Draw colored empty space */
	for (i = left_len + r_len; i < winsize.ws_col; i++) {
		ret = vec_append(ed->buf, " ", 1);
		if (-1 == ret)
			return -1;
	}

	/* Draw the right part */
	ret = vec_append(ed->buf, r, MIN(r_len, winsize.ws_col - left_len));
	if (-1 == ret)
		return -1;

	/* End colored output */
	ret = esc_color_end(ed->buf);
	return ret;
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

static int
ed_flush_buf(struct Ed *const ed)
{
	int ret;

	/* Write buffer to terminal */
	ssize_t len = term_write(vec_items(ed->buf), vec_len(ed->buf));
	if (-1 == len)
		return -1;

	/* Set the length to zero to continue appending characters to the beginning */
	ret = vec_set_len(ed->buf, 0);
	return ret;
}

static int
ed_num_input(struct Ed *const ed, const char digit)
{
	/* Validate digit */
	if (digit < 0 || digit > 9) {
		errno = EINVAL;
		return -1;
	}

	/* Zeroize input if current digit overflows or need to reset */
	if ((SIZE_MAX - digit) / 10 < ed->num_input)
		ed_num_input_clr();
	else
		ed->num_input = (ed->num_input * 10) + digit;
	return 0;
}

static void
ed_num_input_clr(struct Ed *const ed)
{
	ed->num_input = 0;
}

static int
ed_ins_char(struct Ed *const ed, const char ch)
{
	int ret;

	if (!isprint(ch) && '\t' != ch)
		return 0;

	/* Insert character to window */
	ret = win_ins_char(ed->win, ch);
	if (-1 == ret)
		return -1;

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	return 0;
}

static int
ed_ins_empty_line_below(struct Ed *const ed)
{
	int ret = win_ins_empty_line_below(ed->win, ed_repeat_times(ed));
	if (-1 == ret)
		return -1;

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	/* Switch mode for comfort */
	ed->mode = MODE_INS;
	return 0;
}

static int
ed_ins_empty_line_on_top(struct Ed *const ed)
{
	int ret = win_ins_empty_line_on_top(ed->win, ed_repeat_times(ed));
	if (-1 == ret)
		return -1;

	/* Set quit presses count after file change */
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
	/* Switch mode for comfort */
	ed->mode = MODE_INS;
	return 0;
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
	int ret;

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
	ed_num_input_clr(ed);
	/* Set zero length to search query */
	ed_search_input_clr(ed);
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

static int
ed_proc_arrow_key(struct Ed *const ed, const char *const seq, const size_t len)
{
	int ret;
	enum ArrowKey key;
	const size_t repeat_times = ed_repeat_times(ed);
	int (*proc)(Win *, size_t);

	/* Try to extract arrow key */
	ret = esc_extr_arrow_key(seq, len, &key);
	if (-1 == ret) {
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
	return ret;
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

static int
ed_proc_mouse_wh_key(
	struct Ed *const ed,
	const char *const seq,
	const size_t len
) {
	int ret;
	enum MouseWhKey key;
	const size_t repeat_times = ed_repeat_times(ed);
	int (*proc)(Win *, size_t);

	/* Try to extract mouse wheel key */
	ret = esc_extr_mouse_wh_key(seq, len, &key);
	if (-1 == ret) {
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
	return ret;
}

static void
ed_proc_norm_key(struct Ed *const ed, const char key)
{
	int ret;

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
	ret = ed_num_input(ed, key - '0');
	if (-1 == ret) {
		/* Clear input if pressed key is not a digit */
		errno = 0;
		ed_num_input_clr();
	}
}

static void
ed_proc_search_key(struct Ed *const ed, const char key)
{
	int ret;

	switch (key) {
	case CFG_KEY_MODE_SEARCH_TO_NORM:
		win_search(ed->win, ed->search_input, DIR_FWD);
		/* FALLTHROUGH */
	case CFG_KEY_MODE_SEARCH_TO_NORM_CANCEL:
		ed_switch_mode(ed, MODE_NORM);
		break;
	case CFG_KEY_SEARCH_DEL_CHAR:
		ed_search_input_del_char(ed);
		break;
	default:
		ret = ed_search_input(ed, key);
		if (-1 == ret)
			errno = 0;
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
	return ret;
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

static int
ed_search_input(struct Ed *const ed, const char ch)
{
	/* Validate character */
	if (!isprint(ch)) {
		errno = EINVAL;
		return -1;
	}

	/* Write new character if there is place for character and null byte */
	if (ed->search_input_len + 1 < sizeof(ed->search_input)) {
		ed->search_input[ed->search_input_len++] = ch;
		ed->search_input[ed->search_input_len] = 0;
	}
	return 0;
}

static void
ed_search_input_clr(struct Ed *const ed)
{
	/* Reset search input */
	ed->search_input_len = 0;
	ed->search_input[0] = 0;
}

static void
ed_search_input_del_char(struct Ed *const ed)
{
	/* Delete last character in the input if exists */
	if (ed->search_input_len > 0)
		ed->search_input[--ed->search_input_len] = 0;
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
		ed_search_input_clr(ed);
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
