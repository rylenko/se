#include "cfg.h"
#include "ed.h"
#include "mode.h"
#include "term.h"

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
ed_key_proc_ins(Ed *const ed, const char key)
{
	(void)ed;
	(void)key;
}

static void
ed_key_proc_seq(Ed *const ed, const char *const seq, const size_t len)
{
	(void)ed;
	(void)seq;
	(void)len;
}

static void
ed_key_proc_norm(Ed *const ed, const char key)
{
	switch (key) {
	case CFG_KEY_QUIT:
		ed_on_quit_press(ed);
		return;
	case CFG_KEY_SAVE:
		ed_save(ed);
		return;
	case CFG_KEY_SAVE_TO_SPARE_DIR:
		ed_save_to_spare_dir(ed);
	}

	/* Process number input */
	if ('0' <= key && key <= '9')
		ed_input_num(ed, key - '0');
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
