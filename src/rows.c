#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "row.h"
#include "rows.h"

enum {
	/* Memory reallocation step */
	ROWS_REALLOC_STEP = 32,
};

/* Grows rows capacity if needed. */
static void rows_grow_if_needed(Rows *rows);

/* Grows or shrinks the rows capacity. */
static void rows_shrink_if_needed(Rows *rows);

void
rows_break(Rows *const rows, const size_t idx, const size_t col_i)
{
	/* TODO: Copy a smaller portion of a row to the row abome or below */
	Row new_row = row_empty();
	Row *row = &rows->arr[idx];

	/* Fill new row with content if present */
	new_row.len = row->len - col_i;
	if (new_row.len > 0) {
		/* Allocate memory for new row and copy part of previous row to it */
		new_row.cap = new_row.len + 1;
		if (NULL == (new_row.cont = malloc(new_row.cap))) {
			err(EXIT_FAILURE, "Failed to alloc row with capacity %zu", new_row.cap);
		}
		strcpy(new_row.cont, &row->cont[col_i]);
	}
	/* Update render and insert new row */
	row_upd_render(&new_row);
	rows_ins(rows, idx + 1, new_row);

	/* Update pointer because of reallocation in inserting function */
	row = &rows->arr[idx];
	/* Update old row's length and set null byte to break point */
	if (row->len > 0) {
		row->len = col_i;
		row->cont[col_i] = 0;
	}
	/* Update render and shrink capacity if needed for old row */
	row_upd_render(row);
	row_shrink_if_needed(row);
}

void
rows_extend_with_next(Rows *const rows, const size_t idx)
{
	/* Check next row exists */
	if (idx + 1 < rows->cnt) {
		row_extend(&rows->arr[idx], &rows->arr[idx + 1]);
		rows_del(rows, idx + 1);
	}
}

void
rows_del(Rows *const rows, const size_t idx)
{
	/* Validate index */
	assert(idx < rows->cnt);
	/* Free a row */
	row_free(&rows->arr[idx]);
	/* Move other rows if needed */
	if (idx != rows->cnt - 1) {
		memmove(
			&rows->arr[idx],
			&rows->arr[idx + 1],
			sizeof(Row) * (rows->cnt - idx - 1)
		);
	}
	/* Remove from count and check that we need to shrink to fit */
	rows->cnt--;
	rows_shrink_if_needed(rows);
}

void
rows_free(Rows *const rows)
{
	size_t i;
	/* Free rows */
	for (i = 0; i < rows->cnt; i++) {
		row_free(&rows->arr[i]);
	}
	/* Free container */
	free(rows->arr);
	rows->cnt = 0;
	rows->cap = 0;
}

static void
rows_grow_if_needed(Rows *const rows)
{
	if (rows->cnt == rows->cap) {
		rows->cap += ROWS_REALLOC_STEP;
		/* Reallocate with new capacity */
		if (NULL == (rows->arr = realloc(rows->arr, sizeof(Row) * rows->cap))) {
			err(EXIT_FAILURE, "Failed to grow rows to capacity %zu", rows->cap);
		}
	}
}

void
rows_ins(Rows *const rows, const size_t idx, Row row)
{
	/* Validate index */
	assert(idx <= rows->cnt);
	/* Check that we need to grow */
	rows_grow_if_needed(rows);
	/* Move other rows if needed */
	if (idx != rows->cnt) {
		memmove(
			&rows->arr[idx + 1],
			&rows->arr[idx],
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
rows_read(Rows *const rows, FILE *const f)
{
	Row row;
	while (row_read(&row, f)) {
		row_upd_render(&row);
		rows_ins(rows, rows->cnt, row);
	}
}

static void
rows_shrink_if_needed(Rows *const rows)
{
	if (rows->cnt + ROWS_REALLOC_STEP <= rows->cap) {
		rows->cap = rows->cnt;
		/* Reallocate with new capacity */
		if (NULL == (rows->arr = realloc(rows->arr, sizeof(Row) * rows->cap))) {
			err(EXIT_FAILURE, "Failed to shrink rows to capacity %zu", rows->cap);
		}
	}
}

size_t
rows_write(Rows *const rows, FILE *const f)
{
	size_t len = 0;
	size_t row_i;
	for (row_i = 0; row_i < rows->cnt; row_i++) {
		len += row_write(&rows->arr[row_i], f);
	}
	return len;
}
