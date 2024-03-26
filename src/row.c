#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "math.h"
#include "row.h"

enum {
	/* Realloc step, when no capacity or there is too much unused capacity */
	ROW_REALLOC_STEP = 128,
};

/* Reallocates row with new capacity. */
static void row_realloc(Row *const row, const size_t new_cap);

void
row_del(Row *const row, const size_t idx)
{
	/* Validate index */
	assert(idx < row->len);

	/* Move other content left. Do not forget about null byte */
	memmove(&row->cont[idx], &row->cont[idx + 1], row->len - idx);
	/* Decrease length a shrink capacity if there is too much unused space */
	row->len--;
	row_shrink(row, 0);
}

void
row_extend(Row *const dest, const Row *const src)
{
	if (src->len > 0) {
		/* Grow capacity if needed to store source content */
		row_grow_if_needed(dest, dest->len + src->len);

		/* Copy source content to destinationh */
		strcpy(&dest->cont[dest->len], src->cont);
		/* Update length of extended row */
		dest->len += src->len;
	}
}

void
row_free(Row *const row)
{
	/* Free content and render */
	free(row->cont);
	free(row->render);
}

void
row_grow_if_needed(Row *const row, size_t new_len)
{
	/* Grow row in there is no capacity to insert characters */
	if (new_len + 1 > row->cap)
		/* Add specified step or use huge length as new capacity */
		row_realloc(row, MAX(row->cap + ROW_REALLOC_STEP, new_len + 1));
}

void
row_init(Row *const row)
{
	memset(row, 0, sizeof(*row));
}

void
row_ins(Row *const row, const size_t idx, const char ch)
{
	/* Validate index */
	assert(idx <= row->len);

	/* Grow row's capacity if there is no space */
	row_grow_if_needed(row, row->len + 1);
	/* Move content to free new character's space. Do not forget null byte */
	memmove(&row->cont[idx + 1], &row->cont[idx], row->len - idx + 1);

	/* Write new character to string */
	row->cont[idx] = ch;
	/* Increase the length if it is not a zero byte appended to the end */
	if (!(0 == ch && idx == row->len)) {
		row->len++;
	}
}

Row*
row_read(Row *const row, FILE *const f)
{
	int ch;

	/* Initialize row with default values */
	row_init(row);

	/* Read characters */
	while (1) {
		/* Try to read character */
		ch = fgetc(f);
		if (ferror(f) != 0)
			err(EXIT_FAILURE, "Failed to read row's character");

		if (0 == row->len) {
			/* Return early without memory alloc if row is empty or EOF reached */
			if (EOF == ch)
				return NULL;
			else if ('\n' == ch)
				return row;

			/* Insert null byte first to set end of row */
			row_ins(row, row->len, 0);
		}

		/* Check end of row reached */
		if ('\n' == ch || EOF == ch)
			break;
		/* Insert readed character */
		row_ins(row, row->len, ch);
	}

	/* Shrink row's content to fit */
	row_shrink(row, 1);

	/* Render row */
	row_render(row);
	return row;
}

void
row_realloc(Row *const row, size_t new_cap)
{
	/* Reallocate row's content with new capacity */
	if (NULL == (row->cont = realloc(row->cont, new_cap)))
		err(EXIT_FAILURE, "Failed to realloc a row to capacity %zu", new_cap);
	/* Update capacity */
	row->cap = new_cap;
}

void
row_render(Row *const row)
{
	size_t i;
	size_t tabs_cnt = 0;

	/* No content to render */
	if (row->len == 0) {
		return;
	}

	/* Free old render */
	free(row->render);
	row->render_len = 0;

	/* Calculate tabs count */
	for (i = 0; i < row->len; i++)
		if ('\t' == row->cont[i])
			tabs_cnt++;

	/* Allocate render buffer */
	row->render = malloc(row->len + (CFG_TAB_SIZE - 1) * tabs_cnt + 1);
	if (NULL == row->render)
		err(EXIT_FAILURE, "Failed to allocate row's render");

	/* Render content */
	for (i = 0; i < row->len; i++) {
		if ('\t' == row->cont[i]) {
			/* Expand tab with spaces */
			row->render[row->render_len++] = ' ';
			while (row->render_len % CFG_TAB_SIZE != 0)
				row->render[row->render_len++] = ' ';
		} else {
			/* Render simple character */
			row->render[row->render_len++] = row->cont[i];
		}
	}
	/* Do not forget to append null byte */
	row->render[row->render_len] = 0;
}

void
row_shrink(Row *const row, const char hard)
{
	if (0 == row->len && row->cap > 0)
		/* Free allocated memory if row becomes empty */
		row_free(row);
	else if (
		/* Reallocate row's content to equate capacity to length */
		(hard && row->len + 1 < row->cap)
		/* Reallocate if there is too much unused capacity */
		|| row->len + ROW_REALLOC_STEP <= row->cap
	)
		row_realloc(row, row->len + 1);
}

size_t
row_write(Row *const row, FILE *const f)
{
	/* Write row's content and check returned value */
	const size_t len = fwrite(row->cont, sizeof(char), row->len, f);
	if (len != row->len)
		err(EXIT_FAILURE, "Failed to write a row with length %zu", row->len);
	else if (fputc('\n', f) == EOF)
		err(EXIT_FAILURE, "Failed to write \n after row with length %zu", row->len);

	/* Return written length. Do not forget about \n */
	return len + 1;
}
