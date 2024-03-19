#ifndef _ED_H
#define _ED_H

/* winsize */
#include <sys/ioctl.h>
/* Cur */
#include "cur.h"
/* Mode */
#include "mode.h"
/* Rows */
#include "rows.h"

enum {
	ED_MSG_ARR_LEN = 64,
};

/* Structure of editor parameters. */
typedef struct {
	Cur cur;
	char is_dirty;
	Mode mode;
	char msg[ED_MSG_ARR_LEN];
	/* Maximum of `size_t` if not set */
	size_t num_input;
	size_t offset_col;
	size_t offset_row;
	char *path;
	unsigned char quit_presses_rem;
	/* There is always at least 1 row */
	Rows rows;
	struct winsize win_size;
} Ed;

/* Inputs digit to number. */
void ed_input_num(Ed *, unsigned char);

/* Sets message in the editor */
void ed_msg_set(Ed *, const char *, ...);

/* Use this if file was changed. */
void ed_on_f_ch(Ed *);

/* Opens a file in the editor */
Ed ed_open(const char *);

#endif /* _ED_H */
