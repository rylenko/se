#include <stdio.h>
#include <string.h>
#include "row.h"
#include "rows.h"

enum {
	/* Realloc step, when no capacity or there is too much unused capacity */
	ROWS_REALLOC_STEP = 32,
};

/* Reallocates rows container's capacity. */
static void rows_realloc(Rows *const rows, const size_t new_cap);

void
rows_absorb_next(Rows *const rows, const size_t idx)
{
	/* Check that next row exists */
	if (idx + 1 < rows->cnt) {
		/* Extend specified row with next row */
		row_extend(&rows->arr[idx], &rows->arr[idx + 1]);

		/* Delete absorbed row */
		rows_del(rows, idx + 1);
	}
}

void
rows_break(Rows *const rows, const size_t idx, const size_t pos)
{
	Row new_row;
	Row *const row = &rows->arr[idx];

	/* Update new row's length */
	new_row.len = row->len - pos;

	/* Copy content to new row if it is not empty */
	if (new_row.len > 0) {
		/* Set minimum capacity */
		new_row.cap = new_row.len + 1;

		/* Try to allocate space for new row's contnet */
		if (NULL == (new_row.cont = malloc(new_row.cap)))
			err(EXIT_FAILURE, "Failed to alloc new row with capacity %zu", new_row.cap);

		/* Copy content to new row */
		strcpy(new_row.cont, &row->cont[pos]);
	}

	/* Update row if it makes sense. Otherwise it will be freed when shrinking */
	if (row->len > 0) {
		/* Update row's length */
		row->len = pos;
		/* Terminate broken row's content with null byte at break point */
		row->cont[pos] = 0;

		/* Render row with new length */
		row_render(row);
	}
	/* Shrink broken row's capacity */
	row_shrink(row, 0);

	/* Render new row */
	row_render(&new_row);
	/* Insert new row */
	rows_ins(rows, idx + 1, new_row);
}

void
rows_del(Rows *const rows, const size_t idx)
{
	/* Validate index */
	assert(idx < rows->cnt);

	/* Free deleted row */
	row_free(&rows->arr[idx]);

	/* Move other rows if the deleted row is not the last one */
	if (idx < rows->cnt - 1)
		memmove(
			&rows->arr[idx],
			&rows->arr[idx + 1],
			sizeof(*rows->arr) * (rows->cnt - idx - 1)
		);

	/* Decrease rows count and shrink capacity if benefit */
	if (--rows->cnt + ROWS_REALLOC_STEP <= rows->cap)
		rows_realloc(rows, rows->cnt);
}

void
rows_free(Rows *const rows)
{
	/* Free rows and its container */
	while (rows->cnt-- > 0)
		row_free(&rows->arr[i]);
	free(rows->arr);

	/* Zeroize instance */
	memset(rows, 0, sizeof(*rows));
}

static void
rows_realloc(Rows *const rows, const size_t new_cap)
{
	rows->cap = new_cap;
	/* Try to reallocate rows container with new capacity */
	if (NULL == (rows->arr = realloc(rows->arr, sizeof(Row) * rows->cap)))
		err(EXIT_FAILURE, "Failed to realloc rows with capacity %zu", rows->cap);
}

void
rows_init(Rows *const rows)
{
	memset(rows, 0, sizeof(*rows));
}

void
rows_ins(Rows *const rows, const size_t idx, Row row)
{
	/* Validate index */
	assert(idx <= rows->cnt);

	/* Check that we need to grow capacity */
	if (rows->cnt == rows->cap)
		rows_realloc(rows, rows->cap + ROWS_REALLOC_STEP);
	/* Move other rows to free space for new row */
	if (idx != rows->cnt)
		memmove(
			&rows->arr[idx + 1],
			&rows->arr[idx],
			sizeof(*rows->arr) * (rows->cnt - idx)
		);

	/* Insert new row */
	rows->arr[idx] = row;
	rows->cnt++;
}

void
rows_read(Rows *const rows, FILE *const f)
{
	Row row;
	/* Read rows until EOF */
	while (row_read(&row, f) != NULL)
		/* Insert readed row */
		rows_ins(rows, rows->cnt, row);
}

size_t
rows_write(const Rows *const rows, FILE *const f)
{
	size_t i;
	size_t len = 0;
	/* Write rows and collect written length */
	for (i = 0; i < rows->cnt; i++)
		len += row_write(&rows->arr[row_i], f);
	return len;
}
