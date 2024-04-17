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
#include "vec.h"

enum {
	/* Realloc step, when no capacity or there is too much unused capacity */
	LINE_REALLOC_STEP = 128,
};

/* Line of the opened file. */
struct Line {
	Vec *cont; /* Raw content of the line. Does not contain '\n' or '\0' */
	char *render; /* Rendered version of the content */
	size_t render_len; /* Length of rendered content */
};

/* Internal information about the open file. */
struct File {
	char *path; /* Path of readed file. This is where the default save occurs */
	char is_dirty; /* The file has been modified and not saved */
	Vec *lines; /* Lines of file. There is always at least one line */
};

/* Reads lines from the file. */
static void file_read(struct File *, FILE *);

/*
Writes lines to the file.

Returns written bytes count.
*/
static size_t file_write(const struct File *, FILE *);

/* Frees allocated line's buffer. */
static void line_free(struct Line *);

/* Initializes line with zeros. Do not forget to free it. */
static void line_init(struct Line *);

/*
Reads a line from a file without `'\n'`. Returns `NULL` if `EOF` is reached.
*/
static struct Line *line_read(struct Line *, FILE *);

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
Writes a line to the file with `'\n'` at the end.

Returns written bytes count.
*/
static size_t line_write(const struct Line *, FILE *);

void
file_absorb_next_line(struct File *const file, const size_t idx)
{
	struct Line *const dest = vec_get(file->lines, idx);
	struct Line *const src = vec_get(file->lines, idx + 1);

	/* Extending with empty line is useless */
	if (vec_len(src->cont) > 0) {
		/* Extend specified line with next line and render new content */
		vec_append(dest->cont, vec_items(src->cont), vec_len(src->cont));
		line_render(dest);
	}
	/* Delete absorbed line */
	vec_del(file->lines, idx + 1);

	/* Mark file as dirty */
	file->is_dirty = 1;
}

void
file_break_line(struct File *const file, const size_t idx, const size_t pos)
{
	struct Line *line = vec_get(file->lines, idx);
	const size_t new_len = vec_len(line->cont) - pos;
	struct Line new_line;
	line_init(&new_line);

	if (new_len > 0) {
		/* Copy content to new line */
		vec_append(new_line.cont, vec_get(line->cont, pos), new_len);
		/* Render new line */
		line_render(&new_line);
	}

	/* Insert new line */
	vec_ins(file->lines, idx + 1, &new_line, 1);

	/* Update line pointer after insertion because of possible reallocation */
	line = vec_get(file->lines, idx);

	if (new_len > 0) {
		/* Update broken line's length */
		vec_set_len(line->cont, pos);
		/* Render line with new length */
		line_render(line);
		/* Shrink broken line's capacity if needed */
		vec_shrink(line->cont, 0);
	}

	/* Mark file as dirty because of new line */
	file->is_dirty = 1;
}

void
file_close(struct File *const file)
{
	size_t len = vec_len(file->lines);

	/* Free lines. Remember that there is at least one line */
	while (len-- > 0)
		line_free(vec_get(file->lines, len));
	vec_free(file->lines);

	/* Freeing the path since we cloned it earlier */
	free(file->path);
	/* Free allocated opaque struct */
	free(file);
}

void
file_del_char(struct File *const file, const size_t idx, const size_t pos)
{
	/* Delete character and update render */
	struct Line *const line = vec_get(file->lines, idx);
	vec_del(line->cont, pos);
	line_render(line);

	/* Mark file as dirty */
	file->is_dirty = 1;
}

void
file_del_line(struct File *const file, const size_t idx)
{
	/* Free and delete the line */
	line_free(vec_get(file->lines, idx));
	vec_del(file->lines, idx);
	/* Mark file as dirty because of deleted line */
	file->is_dirty = 1;
}

void
file_ins_char(
	struct File *const file,
	const size_t idx,
	const size_t pos,
	const char ch
) {
	/* Insert character to file and update line's render */
	struct Line *const line = vec_get(file->lines, idx);
	vec_ins(line->cont, pos, &ch, 1);
	line_render(line);

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
	vec_ins(file->lines, idx, &empty_line, 1);
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
	return vec_items(((struct Line *)vec_get(file->lines, idx))->cont);
}

size_t
file_line_len(const struct File *const file, const size_t idx)
{
	return vec_len(((struct Line *)vec_get(file->lines, idx))->cont);
}

const char*
file_line_render(const struct File *const file, const size_t idx)
{
	return ((struct Line *)vec_get(file->lines, idx))->render;
}

size_t
file_line_render_len(const struct File *const file, const size_t idx)
{
	return ((struct Line *)vec_get(file->lines, idx))->render_len;
}

size_t
file_lines_cnt(const struct File *const file)
{
	return vec_len(file->lines);
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
	file->lines = vec_alloc(sizeof(struct Line), 32);

	/* Open file, read lines and close the file */
	if (NULL == (inner_file = fopen(path, "r")))
		err(EXIT_FAILURE, "Failed to open file %s", path);
	file_read(file, inner_file);
	if (fclose(inner_file) == EOF)
		err(EXIT_FAILURE, "Failed to close readed file");

	/* Add empty line if there is no lines */
	if (vec_len(file->lines) == 0) {
		line_init(&empty_line);
		vec_append(file->lines, &empty_line, 1);
	}
	return file;
}

const char*
file_path(const struct File *const file)
{
	return file->path;
}

static void
file_read(struct File *const file, FILE *const inner)
{
	struct Line line;
	/* Read lines until EOF */
	while (line_read(&line, inner) != NULL)
		/* Append readed line */
		vec_append(file->lines, &line, 1);
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
	len = file_write(file, inner);
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
	const char *const fname = basename(file->path);
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
	len = snprintf(path, len, "%s/%s_%s", cfg_spare_save_dir, fname, date);
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
	struct Line *line;

	while (*idx < vec_len(file->lines)) {
		line = vec_get(file->lines, *idx);

		/* Try to search on interated line */
		if (line_search(line, pos, query, dir))
			return 1;

		/* Searching backward and start of file reached */
		if (DIR_BWD == dir && 0 == *idx)
			break;

		/* Move to the beginning of another line */
		*idx += DIR_FWD == dir ? 1 : -1;
		line = vec_get(file->lines, *idx);
		*pos = DIR_FWD == dir ? 0 : vec_len(line->cont);
	}
	return 0;
}

static size_t
file_write(const struct File *const file, FILE *const f)
{
	size_t i;
	size_t len = 0;
	/* Write lines and collect written length */
	for (i = 0; i < vec_len(file->lines); i++)
		len += line_write(vec_get(file->lines, i), f);
	return len;
}

void
line_free(struct Line *const line)
{
	/* Free raw content and render */
	vec_free(line->cont);
	free(line->render);
}

static void
line_init(struct Line *const line)
{
	/* Initialize line */
	line->cont = vec_alloc(sizeof(char), 128);
	line->render = NULL;
	line->render_len = 0;
}

static struct Line*
line_read(struct Line *const line, FILE *const f)
{
	int ch;
	/* Initialize empty line to in which to read */
	line_init(line);

	/* Read characters */
	while (1) {
		/* Try to read character */
		ch = fgetc(f);
		if (ferror(f) != 0)
			err(EXIT_FAILURE, "Failed to read line's character");

		if (vec_len(line->cont) == 0) {
			/* First character is EOF. So there is no more lines */
			if (EOF == ch) {
				line_free(line);
				return NULL;
			}
			/* End of line. Return readed line */
			if ('\n' == ch)
				return line;
		}

		/* Check end of line reached */
		if ('\n' == ch || EOF == ch)
			break;
		/* Append readed character */
		vec_append(line->cont, &ch, 1);
	}

	/* Shrink line's content to fit */
	vec_shrink(line->cont, 1);
	/* Render readed line */
	line_render(line);
	return line;
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
	if (vec_len(line->cont) == 0)
		return;

	/* Calculate tabs count */
	for (i = 0; i < vec_len(line->cont); i++) {
		if (*(char *)vec_get(line->cont, i) == '\t')
			tabs_cnt++;
	}

	/* Allocate render buffer */
	line->render = \
		malloc_err(vec_len(line->cont) + (CFG_TAB_SIZE - 1) * tabs_cnt + 1);

	/* Render content */
	for (i = 0; i < vec_len(line->cont); i++) {
		if (*(char *)vec_get(line->cont, i) == '\t') {
			/* Expand tab with spaces */
			line->render[line->render_len++] = ' ';
			while (line->render_len % CFG_TAB_SIZE != 0)
				line->render[line->render_len++] = ' ';
		} else {
			/* Render simple character */
			line->render[line->render_len++] = *(char *)vec_get(line->cont, i);
		}
	}
}

static char
line_search(
	struct Line *const line,
	size_t *const idx,
	const char *const query,
	const enum Dir dir
) {
	const char *res = NULL;

	/* Validate accepted index */
	assert(*idx <= vec_len(line->cont));
	/* Check line is empty */
	if (vec_len(line->cont) == 0)
		return 0;

	switch (dir) {
	case DIR_BWD:
		res = strrstr(vec_items(line->cont), query, *idx);
		break;
	case DIR_FWD:
		res = strstr(vec_get(line->cont, *idx), query);
		break;
	}

	/* Check if no results */
	if (NULL == res)
		return 0;
	/* Set result index and return success code */
	*idx = res - (char *)vec_items(line->cont);
	return 1;
}

static size_t
line_write(const struct Line *const line, FILE *const f)
{
	/* Write line's content and check returned value */
	const size_t len = vec_len(line->cont);
	const size_t wrote = fwrite(vec_items(line->cont), sizeof(char), len, f);
	/* Check errors */
	if (wrote != len)
		err(EXIT_FAILURE, "Failed to write line with %zu bytes", len);
	if (fputc('\n', f) == EOF)
		err(EXIT_FAILURE, "Failed to write \n after %zu bytes", len);
	/* Return written length. Do not forget about \n */
	return wrote + 1;
}
