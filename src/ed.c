/* assert */
#include <assert.h>
/* err */
#include <err.h>
/* va_end, va_list, va_start */
#include <stdarg.h>
/* EOF, FILE, fclose, fopen, vsnprintf */
#include <stdio.h>
/* EXIT_FAILURE, NULL */
#include <stdlib.h>
/* CFG_QUIT_PRESSES_REM_AFT_CH */
#include "cfg.h"
/* cur_new */
#include "cur.h"
#include "ed.h"
/* MODE_NORM */
#include "mode.h"
/* SIZE_MAX */
#include "math.h"
/* str_clone */
#include "str.h"
/* rows_new, rows_read, rows_ins */
#include "row.h"

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
ed_msg_set(Ed *const ed, const char *const fmt, ...)
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
