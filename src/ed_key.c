#include <assert.h>
#include <ctype.h>
#include "cfg.h"
#include "ed.h"
#include "ed_del.h"
#include "ed_ins.h"
#include "ed_mv.h"
#include "ed_quit.h"
#include "ed_save.h"
#include "macros.h"
#include "math.h"
#include "raw_key.h"
#include "term.h"

/* Processes arrow key sequence. */
static void ed_proc_arrow_key(Ed *const ed, const char key);

/* Processes inserting key. */
static void ed_proc_ins_key(Ed *const ed, const char key);

/* Processes key sequence. Useful if single key press is several `char`s. */
static void ed_proc_key_seq(Ed *const ed, const char *seq, const size_t len);

/* Processes normal key. */
static void ed_proc_norm_key(Ed *const ed, const char key);

static void
ed_proc_arrow_key(Ed *const ed, const char key)
{
	/* Repititon times */
	size_t times = SIZE_MAX == ed->num_input ? 1 : ed->num_input;
	switch (key) {
	case 'A':
		ed_mv_up(ed, times);
		return;
	case 'B':
		ed_mv_down(ed, times);
		return;
	case 'C':
		ed_mv_right(ed, times);
		return;
	case 'D':
		ed_mv_left(ed, times);
		return;
	}
}

static void
ed_proc_ins_key(Ed *const ed, const char key)
{
	switch (key) {
	/* Breaks current row */
	case CFG_KEY_INS_BREAK:
		ed_ins_break(ed);
		return;
	/* Delete current character */
	case CFG_KEY_DEL:
		ed_del(ed);
		return;
	/* Switch to normal mode */
	case CFG_KEY_MODE_NORM:
		ed->mode = MODE_NORM;
		return;
	}
	/* Check key is printable and insert */
	if (isprint(key)) {
		ed_ins(ed, key);
	}
}

static void
ed_proc_key_seq(Ed *const ed, const char *seq, const size_t len)
{
	/* Arrows */
	if (
		len == 3
		&& RAW_KEY_ESC == seq[0]
		&& '[' == seq[1]
		&& 'A' <= seq[2]
		&& seq[2] <= 'D'
	) {
		ed_proc_arrow_key(ed, seq[2]);
	}
}

static void
ed_proc_norm_key(Ed *const ed, const char key)
{
	/* Repititon times */
	size_t times = SIZE_MAX == ed->num_input ? 1 : ed->num_input;

	/* Handle number input */
	if (raw_key_is_digit(key)) {
		ed_input_num(ed, raw_key_to_digit(key));
		return;
	} else {
		ed->num_input = SIZE_MAX;
	}
	/* Other keys */
	switch (key) {
	case CFG_KEY_DEL_ROW:
		ed_del_row(ed, times);
		return;
	case CFG_KEY_INS_ROW_BELOW:
		REPEAT(times, ed_ins_row_below(ed));
		return;
	case CFG_KEY_INS_ROW_TOP:
		REPEAT(times, ed_ins_row_top(ed));
		return;
	case CFG_KEY_MODE_INS:
		ed->mode = MODE_INS;
		return;
	case CFG_KEY_MV_TO_BEGIN_OF_F:
		ed_mv_begin_of_f(ed);
		return;
	case CFG_KEY_MV_TO_BEGIN_OF_ROW:
		ed_mv_begin_of_row(ed);
		return;
	case CFG_KEY_MV_DOWN:
		ed_mv_down(ed, times);
		return;
	case CFG_KEY_MV_TO_END_OF_F:
		ed_mv_end_of_f(ed);
		return;
	case CFG_KEY_MV_TO_END_OF_ROW:
		ed_mv_end_of_row(ed);
		return;
	case CFG_KEY_MV_LEFT:
		ed_mv_left(ed, times);
		return;
	case CFG_KEY_MV_TO_NEXT_WORD:
		ed_mv_next_word(ed, times);
		return;
	case CFG_KEY_MV_TO_PREV_WORD:
		ed_mv_prev_word(ed, times);
		return;
	case CFG_KEY_MV_RIGHT:
		ed_mv_right(ed, times);
		return;
	case CFG_KEY_MV_UP:
		ed_mv_up(ed, times);
		return;
	case CFG_KEY_SAVE:
		ed_save(ed, NULL);
		return;
	case CFG_KEY_SAVE_TO_SPARE_DIR:
		ed_save_to_spare_dir(ed);
		return;
	case CFG_KEY_TRY_QUIT:
		ed_quit_try(ed);
		return;
	}
}

void
ed_key_wait_and_proc(Ed *const ed)
{
	char key_seq[3];
	size_t key_seq_len;
	/* Assert that we do not need to quit */
	assert(!ed_quit_done(ed));
	/* Wait key sequence */
	key_seq_len = term_wait_key_seq(key_seq, sizeof(key_seq));
	/* Process entire key sequence if there is more than one `char` */
	if (key_seq_len > 1) {
		ed_proc_key_seq(ed, key_seq, key_seq_len);
		return;
	}
	/* Process single `char` key press with specific mode */
	switch (ed->mode) {
	case MODE_NORM:
		ed_proc_norm_key(ed, key_seq[0]);
		return;
	case MODE_INS:
		ed_proc_ins_key(ed, key_seq[0]);
		return;
	}
}
