#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "cfg.h"
#include "ed.h"
#include "file.h"
#include "math.h"
#include "mode.h"

void
ed_close(Ed *const ed)
{
	/* Close opened file */
	file_close(&ed->file);
	/* Deinitialize window */
	win_deinit(&ed->win);
}

void
ed_input_num(Ed *const ed, const unsigned char digit)
{
	/* Validate digit */
	assert(digit < 10);

	/* Prepare for first digit if input is not set */
	if (SIZE_MAX == ed->num_input)
		ed->num_input = 0;

	/* Zeroize input if current digit overflows. Otherwise add digit */
	if ((SIZE_MAX - digit) / 10 <= ed->num_input)
		ed->num_input = SIZE_MAX;
	else
		ed->num_input = (ed->num_input * 10) + digit;
}

void
ed_on_file_ch(Ed *const ed)
{
	/* Mark file as dirty and set quit presses count */
	ed->is_dirty = 1;
	ed->quit_presses_rem = CFG_DIRTY_FILE_QUIT_PRESSES_CNT;
}

void
ed_open(Ed *const ed, const char *const path, const int ifd, const int ofd)
{
	/* Open file */
	file_open(&ed->file, path);
	/* Initialize window with accepted file descriptors */
	win_init(&ed->win, ifd, ofd);

	/* Set default editting mode */
	ed->mode = MODE_NORM;
	/* Set zero length to message */
	ed->msg[0] = 0;
	/* Make number input inactive */
	ed->num_input = SIZE_MAX;
	/* File is not dirty by default so we may quit using one key press */
	ed->quit_presses_rem = 1;
}

void
ed_set_msg(Ed *const ed, const char *const fmt, ...)
{
	/* Collect arguments and print formatted message to field */
	va_list args;
	va_start(args, fmt);
	vsnprintf(ed->msg, sizeof(ed->msg), fmt, args);
	va_end(args);
}
