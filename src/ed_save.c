/* err, errx */
#include <err.h>
/* errno  */
#include <errno.h>
/* basename */
#include <libgen.h>
/* EOF, FILE, fclose, fflush, fopen, NULL */
#include <stdio.h>
/* EXIT_FAILURE */
#include <stdlib.h>
/* strerror */
#include <string.h>
/* localtime, strftime, time, time_t, tm */
#include <time.h>
/* CFG_SPARE_SAVE_PATH_MAX_LEN, cfg_spare_save_dir */
#include "cfg.h"
/* Ed, ed_msg_set */
#include "ed.h"
#include "ed_save.h"
/* rows_write */
#include "row.h"

static const char *const ed_save_msg_fmt = "%zu bytes saved.";
static const char *const ed_save_fail_msg_fmt = "Failed to save: %s.";

void
ed_save(Ed *const ed, const char *const path)
{
	FILE *f;
	size_t len;

	/* Open file, write rows, flush and close file  */
	if (NULL == (f = fopen(path ? path : ed->path, "w"))) {
		ed_msg_set(ed, ed_save_fail_msg_fmt, strerror(errno));
		return;
	}
	len = rows_write(&ed->rows, f);
	if (fflush(f) == EOF) {
		err(EXIT_FAILURE, "Failed to flush saved file");
	} else if (fclose(f) == EOF) {
		err(EXIT_FAILURE, "Failed to close saved file");
	}
	/* Remove dirty flag and set message */
	ed->is_dirty = 0;
	ed->quit_presses_rem = 1;
	ed_msg_set(ed, ed_save_msg_fmt, len);
}

void
ed_save_to_spare_dir(Ed *ed)
{
	time_t utc;
	struct tm *local;
	char date[15];
	char path[CFG_SPARE_SAVE_PATH_MAX_LEN] = {0};

	/* Get date */
	if ((utc = time(NULL)) == (time_t) - 1) {
		err(EXIT_FAILURE, "Failed to get time to save to spare dir");
	} else if (NULL == (local = localtime(&utc))) {
		err(EXIT_FAILURE, "Failed to get local time to save to spare dir");
	} else if (strftime(date, sizeof(date), "%m-%d_%H-%M-%S", local) == 0) {
		errx(EXIT_FAILURE, "Failed to convert time to string.");
	}
	/* Build full path and save */
	snprintf(
		path,
		sizeof(path),
		"%s/%s_%s",
		cfg_spare_save_dir,
		basename(ed->path),
		date
	);
	ed_save(ed, path);
}
