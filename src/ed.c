#include <assert.h>
#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "cfg.h"
#include "cur.h"
#include "ed.h"
#include "mode.h"
#include "math.h"
#include "str.h"
#include "row.h"

Cur
ed_expand_cur(const Ed *const ed)
{
	size_t i;
	size_t x = 0;
	const Row *const row = &ed->rows.arr[ed->offset_row + ed->cur.y];

	for (i = ed->offset_col; i < ed->offset_col + ed->cur.x; i++, x ++) {
		/* Calculate tab offset */
		if (row->cont[i] == '\t') {
			x += CFG_TAB_SIZE - x % CFG_TAB_SIZE - 1;
		}
	}
	return cur_new(x, ed->cur.y);
}

void
ed_fix_cur(Ed *const ed)
{
	const Row *row;
	size_t f_col_i;
	size_t col_diff;

	/* Clamp cursor on the window */
	ed->cur.y = MIN(ed->cur.y, ed->win_size.ws_row - 2);
	ed->cur.x = MIN(ed->cur.x, ed->win_size.ws_col - 1);

	/* Need to be after cursor clamping to work with fresh values */
	row = &ed->rows.arr[ed->offset_row + ed->cur.y];
	f_col_i = ed->offset_col + ed->cur.x;

	/* Fix x coordinate if current row does not has enough length */
	if (f_col_i > row->len) {
		col_diff = f_col_i - row->len;

		if (ed->cur.x < col_diff) {
			/* Return row on the window */
			ed->offset_col -= col_diff - ed->cur.x;
			ed->cur.x = 0;
			/* Show last character of the row if exists */
			if (row->len > 0) {
				ed->offset_col--;
			}
		} else {
			/* Offset the cursor */
			ed->cur.x -= col_diff;
		}
	}
}

void
ed_input_num(Ed *const ed, const unsigned char digit)
{
	assert(digit < 10);
	if (SIZE_MAX == ed->num_input) {
		/* Prepare for first digit in input */
		ed->num_input = 0;
	}
	if ((SIZE_MAX - digit) / 10 <= ed->num_input) {
		/* Too big. Note that `SIZE_MAX` is flag that input is clear */
		ed->num_input = SIZE_MAX;
	} else {
		/* Append digit to input */
		ed->num_input = (ed->num_input * 10) + digit;
	}
}

void
ed_set_msg(Ed *const ed, const char *const fmt, ...)
{
	/* Collect arguments and print formatted message */
	va_list args;
	va_start(args, fmt);
	vsnprintf(ed->msg, sizeof(ed->msg), fmt, args);
	va_end(args);
}

void
ed_on_f_ch(Ed *const ed)
{
	ed->is_dirty = 1;
	ed->quit_presses_rem = CFG_QUIT_PRESSES_REM_AFT_CH;
}

Ed
ed_open(const char *const path)
{
	FILE *f;
	Ed ed;
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
	return ed;
}
