#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cfg.h"
#include "file.h"
#include "row.h"
#include "rows.h"
#include "str.h"

void
file_close(File *const file)
{
	/* Free readed rows */
	rows_free(&file->rows);
	/* Freeing the path since we cloned it earlier */
	free(file->path);
}

void
file_open(File *const file, const char *const path)
{
	Row empty_row;
	FILE *inner_file;

	/* Initialize file */
	file->pos.col = 0;
	file->pos.row = 0;
	rows_init(&file->rows);
	file->is_dirty = 0;
	file->path = str_copy(path, strlen(path));

	/* Open file, read rows and close the file */
	if (NULL == (inner_file = fopen(path, "r")))
		err(EXIT_FAILURE, "Failed to open file %s", path);
	rows_read(&file->rows, inner_file);
	if (fclose(inner_file) == EOF)
		err(EXIT_FAILURE, "Failed to close readed file");

	/* Add empty row if there is no rows */
	if (0 == file->rows.cnt) {
		row_init(&empty_row);
		rows_ins(&file->rows, 0, empty_row);
	}
}

size_t
file_save(File *const file, const char *const path)
{
	FILE *inner;
	size_t len;

	/* Try to open file */
	if (NULL == (inner = fopen(path == NULL ? file->path : path, "w")))
		return 0;
	/* Write rows to opened file */
	len = rows_write(&file->rows, inner);
	/* Flush and close the file */
	if (fflush(inner) == EOF)
		err(EXIT_FAILURE, "Failed to flush saved file");
	else if (fclose(inner) == EOF)
		err(EXIT_FAILURE, "Failed to close saved file");

	/* Remove dirty flag because file was saved */
	file->is_dirty = 0;
	return len;
}

size_t
file_save_to_spare_dir(File *const file, char *const path, size_t len)
{
	char date[15];
	const char *const filename = basename(file->path);
	const struct tm *local;
	time_t utc;

	/* Get timestamp */
	if ((time_t) - 1 == (utc = time(NULL)))
		err(EXIT_FAILURE, "Failed to get timestamp to save to spare dir");
	/* Get local time from timestamp */
	else if (NULL == (local = localtime(&utc)))
		err(EXIT_FAILURE, "Failed to get local time to save to spare dir");
	/* Format local time to string */
	else if (strftime(date, sizeof(date), "%m-%d_%H-%M-%S", local) == 0)
		errx(EXIT_FAILURE, "Failed to convert time to string to save to spare dir.");

	/* Build full spare path */
	len = snprintf(path, len, "%s/%s_%s", cfg_spare_save_dir, filename, date);
	path[len] = 0;

	/* Save file using built path */
	return file_save(file, path);
}
