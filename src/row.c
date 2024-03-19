/* assert */
#include <assert.h>
/* err */
#include <err.h>
/* FILE, ferror, fgetc, fwrite */
#include <stdio.h>
/* EXIT_FAILURE, NULL, free */
#include <stdlib.h>
/* memmove, strcpy */
#include <string.h>
#include "row.h"

enum {
	/* Memory reallocation step */
	ROW_REALLOC_STEP = 128,
};

/* Force row to shrink to fit the capacity. */
static void row_force_shrink(Row *const row);

/* Grows capacity if needed. */
static void row_grow_if_needed(Row *const row);

void
row_del(Row *const row, const size_t idx)
{
	/* Validate index */
	assert(idx < row->len);
	/* Do not forget about null byte */
	memmove(&row->cont[idx], &row->cont[idx + 1], row->len - idx);
	row->len--;
	row_shrink_if_needed(row);
}

Row
row_empty(void)
{
	return (Row){ .cap = 0, .cont = NULL, .len = 0 };
}

void
row_extend(Row *const row, const Row *const with)
{
	const size_t row_old_len = row->len;
	/* Add length and check that we need to grow capacity */
	row->len += with->len;
	row_grow_if_needed(row);
	/* Copy string */
	strcpy(&row->cont[row_old_len], with->cont);
}

static void
row_force_shrink(Row *const row)
{
	const size_t size = row->len + 1;
	if (0 == row->len && row->cap > 0) {
		row_free(row);
	} else if (size < row->cap) {
		/* Shrink if possible */
		if (NULL == (row->cont = realloc(row->cont, size))) {
			err(EXIT_FAILURE, "Failed to force shrink a row to capacity %zu", size);
		}
		row->cap = size;
	}
}

void
row_free(Row *const row)
{
	free(row->cont);
	row->cont = NULL;
	row->cap = 0;
	row->len = 0;
}

static void
row_grow_if_needed(Row *const row)
{
	/* Do not forget to include null byte */
	if (row->len + 1 >= row->cap) {
		row->cap = row->len + 1 + ROW_REALLOC_STEP;
		/* Realloc with new capacity */
		if (NULL == (row->cont = realloc(row->cont, row->cap))) {
			err(EXIT_FAILURE, "Failed to grow a row to capacity %zu", row->cap);
		}
	}
}

void
row_ins(Row *const row, const size_t idx, const char ch)
{
	/* Validate index */
	assert(idx <= row->len);
	/* Check that we need to grow */
	row_grow_if_needed(row);
	/* Do not forget about null byte */
	memmove(&row->cont[idx + 1], &row->cont[idx], row->len - idx + 1);
	/* Write new character */
	row->cont[idx] = ch;
	if (!(0 == ch && idx == row->len)) {
		row->len++;
	}
}

Row*
row_read(Row *const row, FILE *const f)
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
		/* Check end of row */
		if ('\n' == ch || EOF == ch) {
			break;
		}
		/* Add null byte first */
		if (0 == row->len) {
			row_ins(row, row->len, 0);
		}
		row_ins(row, row->len, ch);
	}

	/* Shrink row's content to fit */
	row_force_shrink(row);
	return row;
}

void
row_shrink_if_needed(Row *const row)
{
	if (0 == row->len && row->cap > 0) {
		row_free(row);
	/* Do not forget to include null byte */
	} else if (row->len + 1 + ROW_REALLOC_STEP <= row->cap) {
		row->cap = row->len + 1;
		/* Realloc with new capacity */
		if (NULL == (row->cont = realloc(row->cont, row->cap))) {
			err(EXIT_FAILURE, "Failed to shrink row to capacity %zu", row->cap);
		}
	}
}

size_t
row_write(Row *const row, FILE *const f)
{
	const size_t len = fwrite(row->cont, sizeof(char), row->len, f);
	if (len != row->len) {
		err(EXIT_FAILURE, "Failed to write a row with length %zu", row->len);
	} else if (fputc('\n', f) == EOF) {
		err(EXIT_FAILURE, "Failed to write \n after row with length %zu", row->len);
	}
	return len;
}
