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

	/* Zeroize input if current digit overflows. Otherwise add digit */
	if ((SIZE_MAX - digit) / 10 < ed->num_input)
		ed->num_input = 0;
	else
		ed->num_input = (ed->num_input * 10) + digit;
}

char
ed_need_to_quit(const Ed *const ed)
{
	return 0 == ed->quit_presses_rem;
}

void
ed_on_quit_press(Ed *const ed)
{
	if (ed->quit_presses_rem > 0) {
		/* Decrease remaining quit presses */
		ed->quit_presses_rem--;

		/* Set message with remaining count if no need to quit */
		if (!ed_need_to_quit(ed))
			ed_set_msg(ed, "File is dirty. Presses remain: %hhu", ed->quit_presses_rem);
	}
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
	ed->num_input = 0;
	/* File is not dirty by default so we may quit using one key press */
	ed->quit_presses_rem = 1;
}

void
ed_quit(Ed *const ed)
{
	/* Close opened file */
	file_close(&ed->file);
	/* Deinitialize window */
	win_deinit();
}

void
ed_save(Ed *const ed)
{
	/* Save file to current file path */
	size_t len = file_save(&ed->file, NULL);

	/* Check save failed */
	if (0 == len) {
		ed_set_msg(ed, "Failed to save: %s", strerror(errno));
	} else {
		ed_set_msg(ed, "%zu bytes saved", len);
		/* Update quit presses */
		ed->quit_presses_rem = 1;
	}
}

void
ed_save_to_spare_dir(Ed *const ed)
{
	char path[CFG_SPARE_PATH_MAX_LEN + 1];
	/* Save file to the spare dir */
	size_t len = file_save_to_spare_dir(&ed->file, path, sizeof(path));
	/* Set message */
	ed_set_msg(ed, "%zu bytes saved to %s", len, path);
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
