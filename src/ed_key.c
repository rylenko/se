#include "cfg.h"
#include "ed.h"
#include "ed_mv.h"
#include "mode.h"
#include "term.h"

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
	(void)key;
}

static void
ed_key_proc_ins(Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_MODE_NORM:
		ed->mode = MODE_NORM;
		return;
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
		return;
	case CFG_KEY_QUIT:
		ed_on_quit_press(ed);
		return;
	case CFG_KEY_SAVE:
		ed_save(ed);
		return;
	case CFG_KEY_SAVE_TO_SPARE_DIR:
		ed_save_to_spare_dir(ed);
		return;
	case CFG_KEY_MV_TO_BEGIN_OF_FILE:
		ed_mv_to_begin_of_file(ed);
		return;
	case CFG_KEY_MV_TO_BEGIN_OF_ROW:
		ed_mv_to_begin_of_row(ed);
		return;
	}

	/* Process number input */
	if ('0' <= key && key <= '9') {
		ed_input_num(ed, key - '0');
		return;
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
		return;
	}

	switch (ed->mode) {
	case MODE_NORM:
		/* Process key in normal mode */
		ed_key_proc_norm(ed, seq[0]);
		return;
	case MODE_INS:
		/* Process key in insertion mode */
		ed_key_proc_ins(ed, seq[0]);
		return;
	}
}
