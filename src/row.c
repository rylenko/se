#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
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

static Row*
row_read(Row *row, FILE *f)
{
	size_t cap = 0;
	int ch;

	/* Zeroize row */
	row->cont = NULL;
	row->len = 0;

	/* Read characters */
	while (1) {
		/* Read new character */
		ch = fgetc(f);
		if (ferror(f) != 0) {
			err("Failed to read row's character:");
		}
		if (0 == row->len) {
			/* Return early if starting character is EOF or newline */
			if (EOF == ch) {
				return NULL;
			} else if ('\n' == ch) {
				return row;
			}
		}
		/* Reallocate row with new capacity */
		if (row->len == cap) {
			cap += REALLOC_STEP_ROW;
			if (!(row->cont = realloc(row->cont, cap))) {
				err("Failed to reallocate row with %zu bytes:", cap);
			}
		}
		/* Write readed character */
		if ('\n' == ch || EOF == ch) {
			row->cont[row->len] = 0;
			break;
		}
		row->cont[row->len++] = ch;
	}

	/* Shrink row's content to fit */
	if (!(row->cont = realloc(row->cont, row->len + 1))) {
		err("Failed to shrink a row from %zu to %zu bytes:", cap, row->len + 1);
	}
	return row;
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
	if (idx > rows->cnt) {
		err("Trying to insert a row, which index greater than rows count.");
	}
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
	if (!(rows->arr = realloc(rows->arr, sizeof(Row) * rows->cap))) {
		err("Failed to reallocate rows with capacity %zu:", rows->cap);
	}
}

void
rows_del(Rows *rows, const size_t idx)
{
	/* Validate index */
	if (idx >= rows->cnt) {
		err("Invalid row's index to delete.");
	}
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
