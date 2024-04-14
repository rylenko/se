#include <assert.h>
#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "alloc.h"
#include "cfg.h"
#include "file.h"
#include "math.h"
#include "str.h"

enum {
	/* Realloc step, when no capacity or there is too much unused capacity */
	LINE_REALLOC_STEP = 128,
	/* Realloc step, when no capacity or there is too much unused capacity */
	LINES_REALLOC_STEP = 32,
};

/* Line of the opened file. */
struct Line {
	char *cont; /* Raw content of the line */
	size_t len; /* Length of content */
	size_t cap; /* Capacity of content's buffer */
	char *render; /* Rendered with tabs expansios content */
	size_t render_len; /* Length of rendered content */
};

/* Lines of the file. */
struct Lines {
	struct Line *arr; /* Dynamic array with lines */
	size_t cnt; /* Count of lines */
	size_t cap; /* Reserved capacity for dynamic array */
};

/* Internal information about the open file. */
struct File {
	char *path; /* Path of readed file. This is where the default save occurs */
	char is_dirty; /* The file has been modified and not saved */
	struct Lines lines; /* Lines of file. There is always at least one line */
};

/*
Deletes character from the line.

Does not update the render so you can do it yourself after several operations.
*/
static void line_del_char(struct Line *, size_t);

/*
Extends line with another line's content.

Does not update the render so you can do it yourself after several operations.
*/
static void line_extend(struct Line *, const struct Line *);

/* Frees allocated line's buffer. */
static void line_free(struct Line *);

/*
Grows line's capacity if there is no space for new characters of passed length.
*/
static void line_grow_if_needed(struct Line *, size_t);

/* Initializes line with zeros. Do not forget to free it. */
static void line_init(struct Line *);

/*
Inserts character to line at index.

Does not update the render so you can do it yourself after several operations.
*/
static void line_ins(struct Line *, size_t, char);

/*
Reads a line from a file without `'\n'`. Returns `NULL` if `EOF` is reached.
*/
static struct Line *line_read(struct Line *, FILE *);

/* Reallocates line with new capacity. */
static void line_realloc(struct Line *, size_t);

/*
Renders line's content how it should look in the window and frees old content.
*/
static void line_render(struct Line *);

/*
Searches substring in the line.

Temporarily changes the string if there is backward searching.

Returns 1 if result found and modifies index pointer. Otherwise returns 0.
*/
static char line_search(struct Line *, size_t *, const char *, enum Dir);

/*
Shrinks line's capacity.

If flag is `1`, then function reallocates line with capacity equal to actual
size. Otherwise it will reallocate memory if a lot of capacity is not used.
*/
static void line_shrink(struct Line *, char);

/*
Writes a line to the file with `'\n'` at the end.

Returns written bytes count.
*/
static size_t line_write(struct Line *, FILE *);

/*
Extends specified line with next line, then frees next line.

Does not update the render so you can do it yourself after several operations.
*/
static void lines_absorb_next_line(struct Lines *, size_t);

/* Finds the line by index and breaks it at specified position. */
static void lines_break_line(struct Lines *, size_t, size_t);

/* Deletes line by index. */
static void lines_del_line(struct Lines *, size_t);

/* Frees the lines container. */
static void lines_free(struct Lines *);

/* Initializes lines container. Do not forget to free it. */
static void lines_init(struct Lines *);

/* Inserts new line at index. */
static void lines_ins_line(struct Lines *, size_t, struct Line);

/* Reads lines from the file. */
static void lines_read(struct Lines *, FILE *);

/* Reallocates lines container's capacity. */
static void lines_realloc(struct Lines *, size_t);

/*
Searches query in the lines.

Temporarily changes the lines if there is backward searching.

Returns 1 if result found and modifies index and position pointers. Otherwise
returns 0.
*/
static char lines_search(
	struct Lines *,
	size_t *,
	size_t *,
	const char *,
	const enum Dir
);

/*
Writes lines to the file.

Returns written bytes count.
*/
static size_t lines_write(const struct Lines *, FILE *);

void
file_absorb_next_line(struct File *const file, const size_t idx)
{
	/* Absorb next line and update current line's render */
	lines_absorb_next_line(&file->lines, idx);
	line_render(&file->lines.arr[idx]);
	/* Mark file as dirty */
	file->is_dirty = 1;
}

void
file_break_line(struct File *const file, const size_t idx, const size_t pos)
{
	/* Break line and mark file as dirty */
	lines_break_line(&file->lines, idx, pos);
	file->is_dirty = 1;
}

void
file_close(struct File *const file)
{
	/* Free readed lines */
	lines_free(&file->lines);
	/* Freeing the path since we cloned it earlier */
	free(file->path);
	/* Free allocated opaque struct */
	free(file);
}

void
file_del_char(struct File *const file, const size_t idx, const size_t pos)
{
	/* Delete character and update render */
	line_del_char(&file->lines.arr[idx], pos);
	line_render(&file->lines.arr[idx]);
	/* Mark file as dirty */
	file->is_dirty = 1;
}

void
file_del_line(struct File *const file, const size_t idx)
{
	/* Delete the line and mark file as dirty */
	lines_del_line(&file->lines, idx);
	file->is_dirty = 1;
}

void
file_ins_char(
	struct File *const file,
	const size_t i,
	const size_t pos,
	const char ch
) {
	/* Insert character to file and update line's render */
	line_ins(&file->lines.arr[i], pos, ch);
	line_render(&file->lines.arr[i]);
	/* Mark file as dirty */
	file->is_dirty = 1;
}

void
file_ins_empty_line(struct File *const file, const size_t idx)
{
	/* Initialize empty line */
	struct Line empty_line;
	line_init(&empty_line);

	/* Insert empty line */
	lines_ins_line(&file->lines, idx, empty_line);
	/* Mark file as dirty */
	file->is_dirty = 1;
}

char
file_is_dirty(const struct File *const file)
{
	return file->is_dirty;
}

const char*
file_line_cont(const struct File *const file, const size_t idx)
{
	return file->lines.arr[idx].cont;
}

size_t
file_line_len(const struct File *const file, const size_t idx)
{
	return file->lines.arr[idx].len;
}

const char*
file_line_render(const struct File *const file, const size_t idx)
{
	return file->lines.arr[idx].render;
}

size_t
file_line_render_len(const struct File *const file, const size_t idx)
{
	return file->lines.arr[idx].render_len;
}

size_t
file_lines_cnt(const struct File *const file)
{
	return file->lines.cnt;
}

struct File*
file_open(const char *const path)
{
	struct Line empty_line;
	FILE *inner_file;

	/* Allocate struct */
	struct File *file = malloc_err(sizeof(*file));
	/* Initialize file */
	file->path = str_copy(path, strlen(path));
	file->is_dirty = 0;
	lines_init(&file->lines);

	/* Open file, read lines and close the file */
	if (NULL == (inner_file = fopen(path, "r")))
		err(EXIT_FAILURE, "Failed to open file %s", path);
	lines_read(&file->lines, inner_file);
	if (fclose(inner_file) == EOF)
		err(EXIT_FAILURE, "Failed to close readed file");

	/* Add empty line if there is no lines */
	if (0 == file->lines.cnt) {
		line_init(&empty_line);
		lines_ins_line(&file->lines, 0, empty_line);
	}
	return file;
}

const char*
file_path(const struct File *const file)
{
	return file->path;
}

size_t
file_save(struct File *const file, const char *const path)
{
	FILE *inner;
	size_t len;

	/* Try to open file */
	if (NULL == (inner = fopen(path == NULL ? file->path : path, "w")))
		return 0;
	/* Write lines to opened file */
	len = lines_write(&file->lines, inner);
	/* Flush and close the file */
	if (fflush(inner) == EOF)
		err(EXIT_FAILURE, "Failed to flush saved file");
	if (fclose(inner) == EOF)
		err(EXIT_FAILURE, "Failed to close saved file");

	/* Remove dirty flag because file was saved */
	file->is_dirty = 0;
	return len;
}

size_t
file_save_to_spare_dir(struct File *const file, char *const path, size_t len)
{
	char date[15];
	const char *const filename = basename(file->path);
	const struct tm *local;
	time_t utc;

	/* Get timestamp */
	if ((time_t) - 1 == (utc = time(NULL)))
		err(EXIT_FAILURE, "Failed to get timestamp to save to spare dir");
	/* Get local time from timestamp */
	if (NULL == (local = localtime(&utc)))
		err(EXIT_FAILURE, "Failed to get local time to save to spare dir");
	/* Format local time to string */
	if (strftime(date, sizeof(date), "%m-%d_%H-%M-%S", local) == 0)
		errx(EXIT_FAILURE, "Failed to convert time to string to save to spare dir.");

	/* Build full spare path */
	len = snprintf(path, len, "%s/%s_%s", cfg_spare_save_dir, filename, date);
	path[len] = 0;

	/* Save file using built path */
	return file_save(file, path);
}

char
file_search(
	struct File *const file,
	size_t *const idx,
	size_t *const pos,
	const char *const query,
	const enum Dir dir
) {
	return lines_search(&file->lines, idx, pos, query, dir);
}

static void
line_del_char(struct Line *const line, const size_t idx)
{
	/* Validate index */
	assert(idx < line->len);

	/* Move other content left. Do not forget about null byte */
	memmove(&line->cont[idx], &line->cont[idx + 1], line->len - idx);
	/* Decrease length a shrink capacity if there is too much unused space */
	line->len--;
	line_shrink(line, 0);
}

static void
line_extend(struct Line *const dest, const struct Line *const src)
{
	if (src->len > 0) {
		/* Line capacity if needed to store source content */
		line_grow_if_needed(dest, dest->len + src->len);

		/* Copy source content to destination */
		strncpy(&dest->cont[dest->len], src->cont, src->len);
		/* Update length of extended line */
		dest->len += src->len;
		/* Set null byte */
		dest->cont[dest->len] = 0;
	}
}

void
line_free(struct Line *const line)
{
	/* Free raw content and render */
	free(line->cont);
	free(line->render);

	/*
	In the code we will want to free the line if it becomes empty. Thanks to
	initialization, we will nullify the pointers and avoid the segmentation fault
	during double free.
	*/
	line_init(line);
}

static void
line_grow_if_needed(struct Line *const line, size_t new_len)
{
	/* Grow line in there is no capacity to insert characters */
	if (new_len + 1 > line->cap)
		/* Add specified step or use huge length as new capacity */
		line_realloc(line, MAX(line->cap + LINE_REALLOC_STEP, new_len + 1));
}

static void
line_init(struct Line *const line)
{
	memset(line, 0, sizeof(*line));
}

static void
line_ins(struct Line *const line, const size_t idx, const char ch)
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

static struct Line*
line_read(struct Line *const line, FILE *const f)
{
	int ch;
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
	/* Render readed line */
	line_render(line);
	return line;
}

static void
line_realloc(struct Line *const line, size_t new_cap)
{
	/* Reallocate and update capacity */
	line->cont = realloc_err(line->cont, new_cap);
	line->cap = new_cap;
}

static void
line_render(struct Line *const line)
{
	size_t i;
	size_t tabs_cnt = 0;

	/* Free old render */
	free(line->render);
	line->render = NULL;
	line->render_len = 0;

	/* No content to render */
	if (0 == line->len)
		return;

	/* Calculate tabs count */
	for (i = 0; i < line->len; i++)
		if ('\t' == line->cont[i])
			tabs_cnt++;

	/* Allocate render buffer */
	line->render = malloc_err(line->len + (CFG_TAB_SIZE - 1) * tabs_cnt + 1);

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

static char
line_search(
	struct Line *const line,
	size_t *const idx,
	const char *const query,
	const enum Dir dir
) {
	const char *res = NULL;
	char tmp;

	/* Validate accepted index */
	assert(*idx <= line->len);
	/* Check line is empty */
	if (0 == line->len)
		return 0;

	switch (dir) {
	case DIR_BWD:
		/* Set end of initial line to do not detect results on the right */
		tmp = line->cont[*idx];
		line->cont[*idx] = 0;

		/* Search from the right */
		res = strrstr(line->cont, query);

		/* Reset end character of search area */
		line->cont[*idx] = tmp;
		break;
	case DIR_FWD:
		res = strstr(&line->cont[*idx], query);
		break;
	}

	/* Check if no results */
	if (NULL == res)
		return 0;
	/* Set result index and return success code */
	*idx = res - line->cont;
	return 1;
}

static void
line_shrink(struct Line *const line, const char hard)
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

static size_t
line_write(struct Line *const line, FILE *const f)
{
	/* Write line's content and check returned value */
	const size_t len = fwrite(line->cont, sizeof(char), line->len, f);
	if (len != line->len)
		err(EXIT_FAILURE, "Failed to write a line with length %zu", line->len);
	if (fputc('\n', f) == EOF)
		err(EXIT_FAILURE, "Failed to write \n after line with len %zu", line->len);

	/* Return written length. Do not forget about \n */
	return len + 1;
}

static void
lines_absorb_next_line(struct Lines *const lines, const size_t idx)
{
	/* Check that next line exists */
	if (idx + 1 < lines->cnt) {
		/* Extend specified line with next line */
		line_extend(&lines->arr[idx], &lines->arr[idx + 1]);

		/* Delete absorbed line */
		lines_del_line(lines, idx + 1);
	}
}

static void
lines_break_line(struct Lines *const lines, const size_t idx, const size_t pos)
{
	struct Line new_line;
	struct Line *const line = &lines->arr[idx];

	/* Update new line and set its length */
	line_init(&new_line);
	new_line.len = line->len - pos;

	/* Copy content to new line if it is not empty */
	if (new_line.len > 0) {
		/* Set minimum capacity */
		new_line.cap = new_line.len + 1;
		/* Try to allocate space for new line's content */
		new_line.cont = malloc_err(new_line.cap);
		/* Copy content to new line */
		strncpy(new_line.cont, &line->cont[pos], new_line.len);
		/* Set null byte */
		new_line.cont[new_line.len] = 0;
	}

	/* Update line if it makes sense. Otherwise it will be freed when shrinking */
	if (line->len > 0) {
		/* Update line's length */
		line->len = pos;
		/* Terminate broken line's content with null byte at break point */
		line->cont[pos] = 0;
	}

	/* Render line with new length or just free old render */
	line_render(line);
	/* Shrink broken line's capacity */
	line_shrink(line, 0);

	/* Render new line */
	line_render(&new_line);
	/* Insert new line */
	lines_ins_line(lines, idx + 1, new_line);
}

static void
lines_del_line(struct Lines *const lines, const size_t idx)
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

static void
lines_free(struct Lines *const lines)
{
	/* Free lines */
	while (lines->cnt-- > 0)
		line_free(&lines->arr[lines->cnt]);
	/* Free dynamic array */
	free(lines->arr);
}

static void
lines_init(struct Lines *const lines)
{
	memset(lines, 0, sizeof(*lines));
}

static void
lines_ins_line(
	struct Lines *const lines,
	const size_t idx,
	const struct Line line
) {
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

static void
lines_read(struct Lines *const lines, FILE *const f)
{
	struct Line line;
	/* Read lines until EOF */
	while (line_read(&line, f) != NULL)
		/* Insert readed line */
		lines_ins_line(lines, lines->cnt, line);
}

static void
lines_realloc(struct Lines *const lines, const size_t new_cap)
{
	/* Reallocate and update capacity */
	lines->arr = realloc_err(lines->arr, sizeof(*lines->arr) * new_cap);
	lines->cap = new_cap;
}

static char
lines_search(
	struct Lines *const lines,
	size_t *const idx,
	size_t *const pos,
	const char *const query,
	const enum Dir dir
) {
	while (*idx < lines->cnt) {
		/* Try to search on interated line */
		if (line_search(&lines->arr[*idx], pos, query, dir))
			return 1;

		/* Searching backward and start of file reached */
		if (DIR_BWD == dir && 0 == *idx)
			break;
		/* Move to the beginning of another line */
		*idx += DIR_FWD == dir ? 1 : -1;
		*pos = DIR_FWD == dir ? 0 : lines->arr[*idx].len;
	}
	return 0;
}

static size_t
lines_write(const struct Lines *const lines, FILE *const f)
{
	size_t line_i;
	size_t len = 0;
	/* Write lines and collect written length */
	for (line_i = 0; line_i < lines->cnt; line_i++)
		len += line_write(&lines->arr[line_i], f);
	return len;
}
