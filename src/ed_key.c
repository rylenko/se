#include "cfg.h"
#include "ed.h"
#include "mode.h"
#include "term.h"
#include "win_mv.h"

/* Processes arrow key. Key must be between 'A' and 'D'. */
static void ed_key_proc_arrow(Ed *const ed, const char key);

/* Process key in insertion mode. */
static void ed_key_proc_ins(Ed *const ed, const char key);

/* Processes key sequence. Useful if single key press is several characters */
static void ed_key_proc_seq(
	Ed *const ed,
	const char *const seq,
	const size_t len
);

/* Process key in normal mode. */
static void ed_key_proc_norm(Ed *const ed, const char key);

static void
ed_key_proc_arrow(Ed *const ed, const char key)
{
	(void)ed;
	switch (key) {
	case 'B':
		/* win_mv_down(&ed->win, ed_get_repeat_times(ed)); */
		break;
	case 'C':
		/* win_mv_right(&ed->win, ed_get_repeat_times(ed)); */
		break;
	}
}

static void
ed_key_proc_ins(Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_MODE_NORM:
		ed->mode = MODE_NORM;
		break;
	}
}

static void
ed_key_proc_seq(Ed *const ed, const char *const seq, const size_t len)
{
	/* Arrows */
	if (
		3 == len
		&& '\x1b' == seq[0]
		&& '[' == seq[1]
		&& 'A' <= seq[2]
		&& seq[2] <= 'D'
	)
		ed_key_proc_arrow(ed, seq[2]);
}

static void
ed_key_proc_norm(Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_MODE_INS:
		ed->mode = MODE_INS;
		break;
	case CFG_KEY_QUIT:
		ed_on_quit_press(ed);
		break;
	case CFG_KEY_SAVE:
		ed_save(ed);
		break;
	case CFG_KEY_SAVE_TO_SPARE_DIR:
		ed_save_to_spare_dir(ed);
		break;
	case CFG_KEY_MV_DOWN:
		/* win_mv_down(&ed->win, ed_get_repeat_times(ed)); */
		break;
	case CFG_KEY_MV_RIGHT:
		/* win_mv_right(&ed->win, ed_get_repeat_times(ed)); */
		break;
	case CFG_KEY_MV_TO_BEGIN_OF_FILE:
		/* win_mv_to_begin_of_file(&ed->win); */
		break;
	case CFG_KEY_MV_TO_BEGIN_OF_LINE:
		win_mv_to_begin_of_line(&ed->win);
		break;
	case CFG_KEY_MV_TO_END_OF_FILE:
		/* win_mv_to_end_of_file(&ed->win); */
		break;
	case CFG_KEY_MV_TO_END_OF_LINE:
		win_mv_to_end_of_line(&ed->win);
		break;
	}
}

void
ed_key_wait_and_proc(Ed *const ed)
{
	char seq[3];
	size_t seq_len;

	/* Wait key press */
	seq_len = term_wait_key(seq, sizeof(seq));

	/* Process key if key is more than one character */
	if (seq_len > 1) {
		ed_key_proc_seq(ed, seq, seq_len);
	} else {
		switch (ed->mode) {
		case MODE_NORM:
			/* Process key in normal mode */
			ed_key_proc_norm(ed, seq[0]);
			break;
		case MODE_INS:
			/* Process key in insertion mode */
			ed_key_proc_ins(ed, seq[0]);
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
