#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include "cfg.h"
#include "ed.h"
#include "file.h"
#include "math.h"
#include "mode.h"

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

char
ed_need_to_quit(const Ed *const ed)
{
	return ed->quit_presses_rem == 0;
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
ed_quit(Ed *const ed)
{
	/* Close opened file */
	file_close(&ed->file);
	/* Deinitialize window */
	win_deinit(&ed->win);
}

void
ed_save(Ed *const ed)
{
	/* Save file to current file path */
	size_t len = file_save(&ed->file, NULL);
	/* Check save failed or not */
	if (len == 0) {
		ed_set_msg(ed, "Failed to open file for save: %s", strerror(errno));
	} else {
		ed_set_msg(ed, "%zu bytes saved", len);
		/* Update quit presses */
		ed->quit_presses_rem = 1;
	}
}

void
ed_save_to_spare_dir(Ed *const ed)
{
	/* Save file to the spare dir */
	size_t len = file_save_to_spare_dir(&ed->file);
	/* Set message */
	ed_set_msg(ed, "%zu bytes saved to spare dir", len);
	/* Update quit presses */
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
