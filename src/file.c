#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cfg.h"
#include "file.h"
#include "line.h"
#include "lines.h"
#include "str.h"

void
file_close(File *const file)
{
	/* Free readed lines */
	lines_free(&file->lines);
	/* Freeing the path since we cloned it earlier */
	free(file->path);
}

void
file_del(File *const file, const size_t idx)
{
	/* Delete the line and mark file as dirty */
	lines_del(&file->lines, idx);
	file->is_dirty = 1;
}

Line*
file_get(const File *const file, const size_t idx)
{
	return lines_get(&file->lines, idx);
}

void
file_open(File *const file, const char *const path)
{
	Line empty_line;
	FILE *inner_file;

	/* Initialize file */
	lines_init(&file->lines);
	file->is_dirty = 0;
	file->path = str_copy(path, strlen(path));

	/* Open file, read lines and close the file */
	if (NULL == (inner_file = fopen(path, "r")))
		err(EXIT_FAILURE, "Failed to open file %s", path);
	lines_read(&file->lines, inner_file);
	if (fclose(inner_file) == EOF)
		err(EXIT_FAILURE, "Failed to close readed file");

	/* Add empty line if there is no lines */
	if (0 == file->lines.cnt) {
		line_init(&empty_line);
		lines_ins(&file->lines, 0, empty_line);
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
	/* Write lines to opened file */
	len = lines_write(&file->lines, inner);
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
