/* snprintf */
#include <stdio.h>
/* basename */
#include <libgen.h>
/* Buf, buf_alloc, buf_free, buf_write, buf_writef */
#include "buf.h"
/* CFG_TAB_SIZE, CFG_COLOR_STAT_BG, CFG_COLOR_STAT_FG */
#include "cfg.h"
/* cur_draw, cur_hide, cur_new, cur_show */
#include "cur.h"
/* Ed */
#include "ed.h"
#include "ed_draw.h"
/* ed_quit_done */
#include "ed_quit.h"
/* MIN, SIZE_MAX */
#include "math.h"
/* mode_str */
#include "mode.h"
/* RawColor, raw_color_begin, raw_color_end */
#include "raw_color.h"
/* term_clr_row_on_right, term_clr_win, term_flush, term_go_home */
#include "term.h"

/* Draws cursor at his position */
static void ed_draw_cur(const Ed *const ed, Buf *const buf);

/* Draws window rows. */
static void ed_draw_rows(const Ed *const ed, Buf *const buf);

/* Draws status on last window row. */
static void ed_draw_stat(Ed *const ed, Buf *const buf);

/* Writes left part of status. */
static size_t ed_draw_stat_left(Ed *ed, Buf *buf);

/* Writes empty space and right part of status. */
static void ed_draw_stat_right(const Ed *ed, Buf *buf, size_t left_len);

void
ed_draw(Ed *const ed)
{
	/* Allocate new buffer and go to start of the window */
	Buf buf = buf_alloc();
	term_go_home(&buf);
	if (ed_quit_done(ed)) {
		/* Clear the window before quit */
		term_clr_win(&buf);
	} else {
		/* Hide cursor */
		cur_hide(&buf);
		/* Write content */
		ed_draw_rows(ed, &buf);
		ed_draw_stat(ed, &buf);
		ed_draw_cur(ed, &buf);
		/* Show the cursor */
		cur_show(&buf);
	}
	/* Flush and free the buffer */
	term_flush(&buf);
	buf_free(&buf);
}

static void
ed_draw_cur(const Ed *const ed, Buf *const buf)
{
	unsigned int i;
	size_t x = 0;
	const Row *const row = &ed->rows.arr[ed->offset_row + ed->cur.y];

	for (i = ed->offset_col; i < ed->offset_col + ed->cur.x; i++, x++) {
		/* Calculate tab offset */
		if (row->cont[i] == '\t') {
			x += CFG_TAB_SIZE - x % CFG_TAB_SIZE - 1;
		}
	}
	/* Write cursor */
	cur_draw(cur_new(x, ed->cur.y), buf);
}

static void
ed_draw_rows(const Ed *const ed, Buf *const buf)
{
	size_t f_row_i;
	const Row *row;
	unsigned short row_i;

	for (row_i = 0; row_i < ed->win_size.ws_row - 1; row_i++) {
		term_clr_row_on_right(buf);

		/* Write row */
		f_row_i = row_i + ed->offset_row;
		if (f_row_i >= ed->rows.cnt) {
			/* No row */
			buf_write(buf, "~", 1);
		} else {
			row = &ed->rows.arr[f_row_i];
			/* This condition also skips empty rows */
			if (row->len > ed->offset_col) {
				buf_write(
					buf,
					row->cont + ed->offset_col,
					MIN(ed->win_size.ws_col, row->len - ed->offset_col)
				);
			}
		}
		buf_write(buf, "\r\n", 2);
	}
}

static void
ed_draw_stat(Ed *const ed, Buf *const buf)
{
	size_t left_len;
	/* Clear row, begin color, write parts and end color */
	term_clr_row_on_right(buf);
	raw_color_begin(
		buf,
		(RawColor)CFG_COLOR_STAT_BG,
		(RawColor)CFG_COLOR_STAT_FG
	);
	left_len = ed_draw_stat_left(ed, buf);
	ed_draw_stat_right(ed, buf, left_len);
	raw_color_end(buf);
}

static size_t
ed_draw_stat_left(Ed *const ed, Buf *const buf)
{
	size_t len;
	/* Write base status to buffer */
	len = buf_writef(buf, " [%s] %s", mode_str(ed->mode), basename(ed->path));
	/* Add mark if file is dirty */
	if (ed->is_dirty) {
		len += buf_write(buf, " [+]", 4);
	}
	/* Write message to buffer if exists */
	if (ed->msg[0]) {
		len += buf_writef(buf, ": %s", ed->msg);
		ed->msg[0] = 0;
	}
	return len;
}

static void
ed_draw_stat_right(const Ed *const ed, Buf *const buf, const size_t left_len)
{
	size_t col_i;
	char coords[32];
	size_t coords_len;
	char num_input[32];
	size_t num_input_len = 0;

	/* Prepare number input if active */
	if (ed->num_input != SIZE_MAX) {
		num_input_len = snprintf(
			num_input,
			sizeof(num_input),
			"%zu << ",
			ed->num_input
		);
	}
	/* Prepare coordinates */
	coords_len = snprintf(
		coords,
		sizeof(coords),
		"%zu, %zu ",
		ed->offset_col + ed->cur.x,
		ed->offset_row + ed->cur.y
	);
	/* Fill colored empty space */
	for (
		col_i = left_len + num_input_len + coords_len;
		col_i < ed->win_size.ws_col;
		col_i++
	) {
		buf_write(buf, " ", 1);
	}
	/* Write right parts and end color */
	buf_write(buf, num_input, num_input_len);
	buf_write(buf, coords, coords_len);
}
