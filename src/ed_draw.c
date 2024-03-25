#include "buf.h"
#include "ed.h"
#include "esc.h"
#include "mode.h"

/* Draws status on last row. */
static size_t ed_draw_stat(Ed *const ed, Buf *const buf);

/* Drows left part of status. */
static size_t ed_draw_stat_left(Ed *const ed, Buf *const buf);

/* Draws right part of status. */
static size_t ed_draw_stat_right(
	Ed *const ed,
	Buf *const buf,
	const size_t left_len
);

void
ed_draw(Ed *const ed)
{
	/* Initialize buffer to write to terminal */
	Buf buf;
	buf_init(&buf);

	/* Go to start of window */
	esc_go_home(&buf);

	if (ed_need_to_quit(ed)) {
		/* Only clear the window on quit */
		esc_clr_win(&buf);
	} else {
		/* Hide cursor to not flicker */
		esc_cur_hide(&buf);

		/* Draw rows of file */
		ed_draw_rows(ed, &buf);
		/* Draw status */
		ed_draw_stat(ed, &buf);
		/* Draw cursor */
		ed_draw_cur(ed, &buf);

		/* Show hidden cursor */
		esc_cur_show(&buf);
	}

	/* Flush and free the buffer */
	buf_flush(&buf);
}

static void
ed_draw_stat(Ed *const ed, Buf *const buf)
{
	size_t left_len = 0;

	/* Clear row before drawing */
	esc_clr_right(buf);

	/* Begin colored output */
	esc_color_begin(buf, CFG_COLOR_STAT_BG, CFG_COLOR_STAT_FG);

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
	char *filename = basename(ed->file.path);
	size_t len = 0;

	/* Write mode and opened file's name */
	len += buf_writef(buf, "%s > %s", mode_str(ed->mode), filename);

	/* Add mark if file is dirty */
	if (ed->is_dirty)
		len += buf_write(buf, " [+]", 4);

	/* Write message if exists */
	if (ed->msg[0]) {
		len += buf_writef(buf, ": %s", ed->msg);
		ed->msg[0] = 0;
	}
	return len;
}

static size_t
ed_draw_stat_right(const Ed *const ed, Buf *const buf, const size_t left_len)
{
	char num_input[32];
	size_t right_len = 0;

	/* Write number input */
	if (ed->num_input != SIZE_MAX)
		right_len += snprintf(num_input, sizeof(num_input), "%zu < ", ed->num_input);

	/* Write current position in the file */
	right_len += snprintf(
		coords,
		sizeof(coords),
		"%zu, %zu ",
		ed->file.pos.col,
		ed->file.pos.row
	);

	/* Write empty space */
	for (i = left_len + right_len; i < ed->win.size.ws_col; i++)
		buf_write(buf, " ", 1);

	/* Write right part */
	buf_writef(buf,
}
