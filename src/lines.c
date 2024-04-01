#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "line.h"
#include "lines.h"

enum {
	/* Realloc step, when no capacity or there is too much unused capacity */
	LINES_REALLOC_STEP = 32,
};

/* Lines of the file. */
struct Lines {
	Line *arr; /* Dynamic array with lines */
	size_t cnt; /* Count of lines */
	size_t cap; /* Reserved capacity for dynamic array */
};

/* Reallocates lines container's capacity. */
static void lines_realloc(Lines *const lines, const size_t new_cap);

void
lines_absorb_next(Lines *const lines, const size_t idx)
{
	/* Check that next line exists */
	if (idx + 1 < lines->cnt) {
		/* Extend specified line with next line */
		line_extend(&lines->arr[idx], &lines->arr[idx + 1]);

		/* Delete absorbed line */
		lines_del(lines, idx + 1);
	}
}

Lines*
lines_alloc(void)
{
	Lines *const lines = calloc(1, sizeof(Lines));
	if (NULL == lines)
		err(EXIT_FAILURE, "Failed to allocate lines");
	return lines;
}

void
lines_break(Lines *const lines, const size_t idx, const size_t pos)
{
	Line new_line;
	Line *const line = &lines->arr[idx];

	/* Update new line's length */
	new_line.len = line->len - pos;

	/* Copy content to new line if it is not empty */
	if (new_line.len > 0) {
		/* Set minimum capacity */
		new_line.cap = new_line.len + 1;

		/* Try to allocate space for new line's contnet */
		if (NULL == (new_line.cont = malloc(new_line.cap)))
			err(EXIT_FAILURE, "Failed to alloc new line with cap %zu", new_line.cap);

		/* Copy content to new line */
		strcpy(new_line.cont, &line->cont[pos]);
	}

	/* Update line if it makes sense. Otherwise it will be freed when shrinking */
	if (line->len > 0) {
		/* Update line's length */
		line->len = pos;
		/* Terminate broken line's content with null byte at break point */
		line->cont[pos] = 0;

		/* Render line with new length */
		line_render(line);
	}
	/* Shrink broken line's capacity */
	line_shrink(line, 0);

	/* Render new line */
	line_render(&new_line);
	/* Insert new line */
	lines_ins(lines, idx + 1, new_line);
}

size_t
lines_cnt(const Lines *const lines)
{
	return lines->cnt;
}

void
lines_del(Lines *const lines, const size_t idx)
{
	/* Validate index */
	assert(idx < lines->cnt);

	/* Free deleted line */
	line_free(&lines->arr[idx]);

	/* Move other lines if the deleted line is not the last one */
	if (idx < lines->cnt - 1)
		memmove(
			&lines->arr[idx],
			&lines->arr[idx + 1],
			sizeof(*lines->arr) * (lines->cnt - idx - 1)
		);

	/* Decrease lines count and shrink capacity if benefit */
	if (--lines->cnt + LINES_REALLOC_STEP <= lines->cap)
		lines_realloc(lines, lines->cnt);
}

void
lines_free(Lines *const lines)
{
	/* Free lines */
	while (lines->cnt-- > 0)
		line_free(&lines->arr[lines->cnt]);
	/* Free dynamic array and opaque struct */
	free(lines->arr);
	free(lines);
}

Line*
lines_get(const Lines *const lines, const size_t idx)
{
	return &lines->arr[idx];
}

static void
lines_realloc(Lines *const lines, const size_t new_cap)
{
	lines->cap = new_cap;
	/* Try to reallocate lines container with new capacity */
	if (NULL == (lines->arr = realloc(lines->arr, sizeof(Line) * lines->cap)))
		err(EXIT_FAILURE, "Failed to realloc lines with capacity %zu", lines->cap);
}

void
lines_ins(Lines *const lines, const size_t idx, Line line)
{
	/* Validate index */
	assert(idx <= lines->cnt);

	/* Check that we need to gline capacity */
	if (lines->cnt == lines->cap)
		lines_realloc(lines, lines->cap + LINES_REALLOC_STEP);
	/* Move other lines to free space for new line */
	if (idx != lines->cnt)
		memmove(
			&lines->arr[idx + 1],
			&lines->arr[idx],
			sizeof(*lines->arr) * (lines->cnt - idx)
		);

	/* Insert new line */
	lines->arr[idx] = line;
	lines->cnt++;
}

void
lines_read(Lines *const lines, FILE *const f)
{
	Line line;
	/* Read lines until EOF */
	while (line_read(&line, f) != NULL)
		/* Insert readed line */
		lines_ins(lines, lines->cnt, line);
}

size_t
lines_write(const Lines *const lines, FILE *const f)
{
	size_t line_i;
	size_t len = 0;
	/* Write lines and collect written length */
	for (line_i = 0; line_i < lines->cnt; line_i++)
		len += line_write(&lines->arr[line_i], f);
	return len;
}
