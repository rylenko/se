#include <libgen.h>
#include <signal.h>
#include "buf.h"
#include "cfg.h"
#include "ed.h"
#include "esc.h"
#include "mode.h"
#include "term.h"
#include "win_draw.h"

/* Draws status on last row. */
static void ed_draw_stat(Ed *const ed, Buf *const buf);

/* Drows left part of status. */
static size_t ed_draw_stat_left(Ed *const ed, Buf *const buf);

/* Draws right part of status. */
static void ed_draw_stat_right(
	const Ed *const ed,
	Buf *const buf,
	const size_t left_len
);

void
ed_draw(Ed *const ed)
{
	/* Initialize buffer to write to terminal */
	Buf buf;
	buf_init(&buf);

	/* Go to start of window and clear the window */
	esc_go_home(&buf);
	esc_clr_win(&buf);

	if (!ed_need_to_quit(ed)) {
		/* Hide cursor to not flicker */
		esc_cur_hide(&buf);

		/* Draw lines of file */
		win_draw_lines(&ed->win, &buf);
		/* Draw status */
		ed_draw_stat(ed, &buf);
		/* Draw expanded cursor */
		win_draw_cur(&ed->win, &buf);

		/* Show hidden cursor */
		esc_cur_show(&buf);
	}

	/* Flush and free the buffer */
	term_flush(&buf);
}

void
ed_draw_handle_signal(Ed *const ed, const int signal)
{
	/* Handle signals for window */
	win_handle_signal(&ed->win, signal);

	/* Handle window size change signal */
	if (SIGWINCH == signal)
		/* Redraw after window resizing */
		ed_draw(ed);
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
	char *filename = basename(ed->win.file.path);
	size_t len = 0;

	/* Write mode and opened file's name */
	len += buf_writef(buf, " %s > %s", mode_str(ed->mode), filename);

	/* Add mark if file is dirty */
	if (ed->win.file.is_dirty)
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

	/* Write number input */
	if (ed->num_input > 0)
		right_len += snprintf(num_input, sizeof(num_input), "%zu < ", ed->num_input);

	/* Write current position in the file */
	right_len += snprintf(
		pos,
		sizeof(pos),
		"%zu, %zu ",
		ed->win.curr_line_cont_idx,
		ed->win.curr_line_idx
	);

	/* Write empty space */
	for (col = left_len + right_len; col < ed->win.size.ws_col; col++)
		buf_write(buf, " ", 1);

	/* Write right part */
	buf_writef(buf, "%s%s", num_input, pos);
}
