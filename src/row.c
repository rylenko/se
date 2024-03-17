#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "row.h"

/* Memory reallocation steps */
typedef enum {
	REALLOC_STEP_ROWS = 32,
	REALLOC_STEP_ROW = 128,
} ReallocStep;

/* Frees row's content. */
static void row_free(Row *row);

/*
Reads new row.

Returns pointer to accepted row on success and `NULL` on `EOF`.
*/
static Row *row_read(Row *row, FILE *f);

/* Grows or shrinks the row's capacity if there is a big benefit. */
static void row_realloc_if_needed(Row *row);

/* Shrinks to fit the row. */
static void row_shrink_to_fit(Row *row);

/* Writes row to the file with newline character. */
static size_t row_write(Row *row, FILE *f);

/* Grows or shrinks the rows capacity. */
static void rows_realloc_if_needed(Rows *rows);

Row
row_empty(void)
{
	return (Row){ .cont = NULL, .len = 0 };
}

static void
row_free(Row *row)
{
	free(row->cont);
	row->cont = NULL;
	row->len = 0;
}

void
row_ins(Row *row, const size_t idx, const char ch)
{
	/* Validate index */
	assert(idx <= row->len);
	/* Check that we need to grow */
	row_realloc_if_needed(row);
	/* Always need to move because of null byte */
	memmove(
		row->cont + idx + 1,
		row->cont + idx,
		sizeof(char) * (row->len - idx + 1)
	);
	/* Write new character */
	row->cont[idx] = ch;
	row->len++;
}

static Row*
row_read(Row *row, FILE *f)
{
	int ch;

	/* Zeroize row */
	row->cap = 0;
	row->cont = NULL;
	row->len = 0;

	/* Read characters */
	while (1) {
		/* Read new character */
		ch = fgetc(f);
		if (ferror(f) != 0) {
			err(EXIT_FAILURE, "Failed to read row's character");
		}
		if (0 == row->len) {
			/* Return early if starting character is EOF or newline */
			if (EOF == ch) {
				return NULL;
			} else if ('\n' == ch) {
				return row;
			}
		}
		/* Grow row if needed */
		row_realloc_if_needed(row);
		/* Check end of row */
		if ('\n' == ch || EOF == ch) {
			break;
		}
		/* Add null byte first */
		if (0 == row->len) {
			row->cont[row->len] = 0;
		}
		row_ins(row, row->len, ch);
	}

	/* Shrink row's content to fit */
	row_shrink_to_fit(row);
	return row;
}

static void
row_realloc_if_needed(Row *row)
{
	/* Do not forget to include null byte */
	if (row->len + 1 == row->cap || row->cont == NULL) {
		row->cap += REALLOC_STEP_ROW;
	} else {
		return;
	}
	/* Realloc with new capacity */
	if ((row->cont = realloc(row->cont, row->cap)) == NULL) {
		err(EXIT_FAILURE, "Failed to reallocate row with %zu bytes", row->cap);
	}
}

static void
row_shrink_to_fit(Row *row)
{
	size_t size = row->len + 1;
	if (size < row->cap) {
		/* Shrink if possible */
		if ((row->cont = realloc(row->cont, size)) == NULL) {
			err(EXIT_FAILURE, "Failed to shrink a row %zu -> %zu", row->cap, size);
		}
		row->cap = size;
	}
}

static size_t
row_write(Row *row, FILE *f)
{
	size_t len = fwrite(row->cont, sizeof(char), row->len, f);
	if (len != row->len) {
		err(EXIT_FAILURE, "Failed to write a row with length %zu", row->len);
	} else if (fputc('\n', f) == EOF) {
		err(EXIT_FAILURE, "Failed to write \n after row with length %zu", row->len);
	}
	return len;
}

void
rows_del(Rows *rows, const size_t idx)
{
	/* Validate index */
	assert(idx < rows->cnt);
	/* Free a row */
	row_free(&rows->arr[idx]);
	/* Move other rows if needed */
	if (idx != rows->cnt - 1) {
		memmove(
			rows->arr + idx,
			rows->arr + idx + 1,
			sizeof(Row) * (rows->cnt - idx - 1)
		);
	}
	/* Remove from count and check that we need to shrink to fit */
	rows->cnt--;
	rows_realloc_if_needed(rows);
}

void
rows_free(Rows *rows)
{
	size_t i;

	/* Free rows */
	for (i = 0; i < rows->cnt; i++) {
		row_free(rows->arr + i);
	}
	/* Free container */
	free(rows->arr);
	rows->cnt = 0;
	rows->cap = 0;
}

void
rows_ins(Rows *rows, const size_t idx, Row row)
{
	/* Validate index */
	assert(idx <= rows->cnt);
	/* Check that we need to grow */
	rows_realloc_if_needed(rows);
	/* Move other rows if needed */
	if (idx != rows->cnt) {
		memmove(
			rows->arr + idx + 1,
			rows->arr + idx,
			sizeof(Row) * (rows->cnt - idx)
		);
	}
	/* Write new row */
	rows->arr[idx] = row;
	rows->cnt++;
}

Rows
rows_new(void)
{
	return (Rows){ .arr = NULL, .cap = 0, .cnt = 0 };
}

void
rows_read(Rows *rows, FILE *f)
{
	Row row = row_empty();
	while (row_read(&row, f)) {
		rows_ins(rows, rows->cnt, row);
	}
}

static void
rows_realloc_if_needed(Rows *rows)
{
	if (rows->cnt == rows->cap) {
		rows->cap += REALLOC_STEP_ROWS;
	} else if (rows->cnt + REALLOC_STEP_ROWS <= rows->cap) {
		rows->cap = rows->cnt;
	} else {
		return;
	}
	if ((rows->arr = realloc(rows->arr, sizeof(Row) * rows->cap)) == NULL) {
		err(EXIT_FAILURE, "Failed to reallocate rows with capacity %zu", rows->cap);
	}
}

size_t
rows_write(Rows *rows, FILE *f)
{
	size_t len = 0;
	size_t row_i;
	for (row_i = 0; row_i < rows->cnt; row_i++) {
		len += row_write(rows->arr + row_i, f);
	}
	return len;
}
