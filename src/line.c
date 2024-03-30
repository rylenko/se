#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "line.h"
#include "math.h"

enum {
	/* Realloc step, when no capacity or there is too much unused capacity */
	LINE_REALLOC_STEP = 128,
};

/* Reallocates line with new capacity. */
static void line_realloc(Line *const line, const size_t new_cap);

void
line_del(Line *const line, const size_t idx)
{
	/* Validate index */
	assert(idx < line->len);

	/* Move other content left. Do not forget about null byte */
	memmove(&line->cont[idx], &line->cont[idx + 1], line->len - idx);
	/* Decrease length a shrink capacity if there is too much unused space */
	line->len--;
	line_shrink(line, 0);
}

void
line_extend(Line *const dest, const Line *const src)
{
	if (src->len > 0) {
		/* Line capacity if needed to store source content */
		line_grow_if_needed(dest, dest->len + src->len);

		/* Copy source content to destinationh */
		strcpy(&dest->cont[dest->len], src->cont);
		/* Update length of extended line */
		dest->len += src->len;
	}
}

void
line_free(Line *const line)
{
	/* Free content and render */
	free(line->cont);
	free(line->render);
}

void
line_grow_if_needed(Line *const line, size_t new_len)
{
	/* Grow line in there is no capacity to insert characters */
	if (new_len + 1 > line->cap)
		/* Add specified step or use huge length as new capacity */
		line_realloc(line, MAX(line->cap + LINE_REALLOC_STEP, new_len + 1));
}

void
line_init(Line *const line)
{
	memset(line, 0, sizeof(*line));
}

void
line_ins(Line *const line, const size_t idx, const char ch)
{
	/* Validate index */
	assert(idx <= line->len);

	/* Grow line's capacity if there is no space */
	line_grow_if_needed(line, line->len + 1);
	/* Move content to free new character's space. Do not forget null byte */
	memmove(&line->cont[idx + 1], &line->cont[idx], line->len - idx + 1);

	/* Write new character to string */
	line->cont[idx] = ch;
	/* Increase the length if it is not a zero byte appended to the end */
	if (!(0 == ch && idx == line->len))
		line->len++;
}

Line*
line_read(Line *const line, FILE *const f)
{
	int ch;

	/* Initialize line with default values */
	line_init(line);

	/* Read characters */
	while (1) {
		/* Try to read character */
		ch = fgetc(f);
		if (ferror(f) != 0)
			err(EXIT_FAILURE, "Failed to read line's character");

		if (0 == line->len) {
			/* Return early without memory alloc if line is empty or EOF reached */
			if (EOF == ch)
				return NULL;
			else if ('\n' == ch)
				return line;

			/* Insert null byte first to set end of line */
			line_ins(line, line->len, 0);
		}

		/* Check end of line reached */
		if ('\n' == ch || EOF == ch)
			break;
		/* Insert readed character */
		line_ins(line, line->len, ch);
	}

	/* Shrink line's content to fit */
	line_shrink(line, 1);

	/* Render line */
	line_render(line);
	return line;
}

void
line_realloc(Line *const line, size_t new_cap)
{
	/* Reallocate line's content with new capacity */
	if (NULL == (line->cont = realloc(line->cont, new_cap)))
		err(EXIT_FAILURE, "Failed to realloc a line to capacity %zu", new_cap);
	/* Update capacity */
	line->cap = new_cap;
}

void
line_render(Line *const line)
{
	size_t i;
	size_t tabs_cnt = 0;

	/* No content to render */
	if (0 == line->len)
		return;

	/* Free old render */
	free(line->render);
	line->render_len = 0;

	/* Calculate tabs count */
	for (i = 0; i < line->len; i++)
		if ('\t' == line->cont[i])
			tabs_cnt++;

	/* Allocate render buffer */
	line->render = malloc(line->len + (CFG_TAB_SIZE - 1) * tabs_cnt + 1);
	if (NULL == line->render)
		err(EXIT_FAILURE, "Failed to allocate line's render");

	/* Render content */
	for (i = 0; i < line->len; i++) {
		if ('\t' == line->cont[i]) {
			/* Expand tab with spaces */
			line->render[line->render_len++] = ' ';
			while (line->render_len % CFG_TAB_SIZE != 0)
				line->render[line->render_len++] = ' ';
		} else {
			/* Render simple character */
			line->render[line->render_len++] = line->cont[i];
		}
	}
	/* Do not forget to append null byte */
	line->render[line->render_len] = 0;
}

void
line_shrink(Line *const line, const char hard)
{
	if (0 == line->len && line->cap > 0)
		/* Free allocated memory if line becomes empty */
		line_free(line);
	else if (
		/* Reallocate line's content to equate capacity to length */
		(hard && line->len + 1 < line->cap)
		/* Reallocate if there is too much unused capacity */
		|| line->len + LINE_REALLOC_STEP <= line->cap
	)
		line_realloc(line, line->len + 1);
}

size_t
line_write(Line *const line, FILE *const f)
{
	/* Write line's content and check returned value */
	const size_t len = fwrite(line->cont, sizeof(char), line->len, f);
	if (len != line->len)
		err(EXIT_FAILURE, "Failed to write a line with length %zu", line->len);
	else if (fputc('\n', f) == EOF)
		err(EXIT_FAILURE, "Failed to write \n after line with len %zu", line->len);

	/* Return written length. Do not forget about \n */
	return len + 1;
}
