#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#include "pos.h"
#include "row.h"
#include "rows.h"

void
file_open(File *const file, const char *const path)
{
	Row empty_row;
	FILE *inner_file;

	/* Initialize file */
	pos_init(&file->pos);
	file->is_dirty = 0;
	if (NULL == (file->path = strdup(path))) {
		err(EXIT_FAILURE, "Failed to duplicate file path");
	}

	/* Open file, read rows and close the file */
	if (NULL == (inner_file = fopen(path, "r"))) {
		err(EXIT_FAILURE, "Failed to open file %s", path);
	}
	rows_read(&file->rows, inner_file);
	if (fclose(inner_file) == EOF) {
		err(EXIT_FAILURE, "Failed to close readed file");
	}

	/* Add empty row if there is no rows */
	if (file->rows.cnt == 0) {
		row_init(&empty_row);
		rows_ins(&file->rows, 0, empty_row);
	}
	return ret;
}
