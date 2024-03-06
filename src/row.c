#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "row.h"

/* Memory reallocation steps */
#define ROWS_REALLOC_STEP (32)
#define ROW_REALLOC_STEP (256)

/* Allocates new row. */
static Row row_alloc(void);

/* Frees row's content. */
static void row_free(Row *row);

/*
Reads new row.

Returns pointer to accepted row on success and `NULL` on `EOF`.
*/
static Row *row_read(Row *row, FILE *f);

/* Grows rows capacity. */
static void rows_grow(Rows *rows);

/* Inserts new row at index. */
void rows_ins(Rows *rows, size_t idx, Row row);

static Row
row_alloc(void)
{
	return (Row){ .cont = NULL, .len = 0 };
}

static void
row_free(Row *row)
{
	free(row->cont);
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
		if (EOF == ch && 0 == row->len)
			return NULL;

		/* Reallocate row with new capacity */
		if (row->len == cap) {
			cap += ROW_REALLOC_STEP;
			if (!(row->cont = realloc(row->cont, cap)))
				err("Failed to reallocate row with %zu bytes:", cap);
		}

		/* Write readed character */
		if ('\n' == ch || EOF == ch) {
			row->cont[row->len++] = 0;
			break;
		}
		row->cont[row->len++] = ch;
	}

	/* Shrink row's content to fit */
	if (!(row->cont = realloc(row->cont, row->len)))
		err("Failed to shrink to fit a row from %zu to %zu bytes:", cap, row->len);
	return row;
}

Rows
rows_alloc(void)
{
	/* Allocate and initialize rows container */
	Row *arr = malloc(ROWS_REALLOC_STEP * sizeof(Row));
	if (!arr)
		err("Failed to allocate initial %zu rows:", ROWS_REALLOC_STEP);
	return (Rows){ .arr = arr, .cap = ROWS_REALLOC_STEP, .cnt = 0 };
}

void
rows_free(Rows *rows)
{
	size_t i;

	/* Free rows */
	for (i = 0; i < rows->cnt; i++)
		row_free(rows->arr + i);
	/* Free container */
	free(rows->arr);
	rows->cnt = 0;
	rows->cap = 0;
}

static void
rows_grow(Rows *rows)
{
	rows->cap += ROWS_REALLOC_STEP;
	if (!(rows->arr = realloc(rows->arr, sizeof(Row) * rows->cap)))
		err("Failed to reallocate rows with capacity %zu:", rows->cap);
}

void
rows_ins(Rows *rows, size_t idx, Row row)
{
	/* Validate index */
	if (idx > rows->cnt)
		err("Trying to insert a row, which index greater than rows count.");
	/* Check that we need to grow */
	if (rows->cnt == rows->cap)
		rows_grow(rows);
	/* Move other rows if needed */
	if (idx != rows->cnt)
		/* TODO: Linked list is better for creating new rows */
		memmove(
			rows->arr + idx + 1,
			rows->arr + idx,
			sizeof(Row) * (rows->cnt - idx)
		);
	/* Write new row */
	rows->arr[idx] = row;
	rows->cnt++;
}

void
rows_read(Rows *rows, FILE *f)
{
	Row row = row_alloc();
	while (row_read(&row, f) != NULL)
		rows_ins(rows, rows->cnt, row);
}

/* TODO: create shrink_to_fit for rows_remove */
