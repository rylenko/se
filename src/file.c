#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cfg.h"
#include "file.h"
#include "math.h"
#include "str.h"
#include "vec.h"

enum {
	LINE_CHARS_CAP_STEP = 128, /* line's chars capacity reallocation step */
	FILE_LINES_CAP_STEP = 32, /* file's lines capacity reallocation step */
};

/* line of the opened file. */
struct line {
	Vec *chars; /* Raw content of the line. Does not contain '\n' or '\0' */
	char *render; /* Rendered version of the content */
	size_t render_len; /* Length of rendered content */
};

/* Information about the open file. */
struct file {
	char *path; /* Path of readed file. This is where the default save occurs */
	char is_dirty; /* If set, then the file has unsaved changes */
	Vec *lines; /* lines of file. There is always at least one line */
};

/*
Allocates empty file container. Do not forget to free it.

Returns pointer to opaque struct on succcess and `NULL` on error.
*/
static struct file *file_alloc(const char *);


/* Frees file allocated file. */
static void file_free(struct file *);

/*
Reads lines from the file.

Returns 0 on success and -1 on error. Note that you need to free readed lines
*/
static int file_read(struct file *, FILE *);

/*
Writes lines to the file.

Returns written bytes count on success and 0 on error.
*/
static size_t file_write(const struct file *, FILE *);

/*
Appends passed chars to line and renders updated line.

Returns 0 on success and -1 on error.
*/
static int line_append(struct line *, const char *, size_t);

/*
Cuts a line. The argument specifies how many first chars will remain.

Returns 0 on success and -1 on error.
*/
static int line_cut(struct line *, size_t);

/* Frees allocated line's buffer. */
static void line_free(struct line *);

/*
Initializes line with zeros. Do not forget to free it.

Returns 0 on success and -1 on error.
*/
static int line_init(struct line *);

/*
Reads a line from a file without `'\n'`. Returns `NULL` if `EOF` is reached.

Returns 1 if line readed, 0 if EOF reached and there is no line to read and -1
if error.
*/
static int line_read(struct line *, FILE *);

/*
Renders line chars how it look in the window.

Returns 0 on success and -1 on error.
*/
static int line_render(struct line *);

/*
Searches substring in the line.

Temporarily changes the string if there is backward searching.

Returns 1 if result found, 0 if no result and -1 on error.
*/
static int line_search(struct line *, size_t *, const char *, enum dir);

/*
Writes a line to the file with `'\n'` at the end.

Returns written bytes count on success and 0 on error.
*/
static size_t line_write(const struct line *, FILE *);

int
file_absorb_next_line(struct file *const file, const size_t idx)
{
	int ret;
	struct line next;
	struct line *curr;

	/* Remove next line */
	ret = vec_remove(file->lines, idx + 1, &next);
	if (-1 == ret)
		return -1;

	/* Append current line with next line's chars if next line is not empty */
	if (vec_len(next.chars) > 0) {
		/* Get current line here because vector may realloc after removing */
		curr = vec_get(file->lines, idx);
		if (NULL == curr)
			goto ret_free;

		ret = line_append(curr, vec_items(next.chars), vec_len(next.chars));
		if (-1 == ret)
			goto ret_free;
	}

	/* Mark file as dirty */
	file->is_dirty = 1;
ret_free:
	line_free(&next);
	return ret;
}

static struct file*
file_alloc(const char *const path)
{
	struct file *file;

	/* Allocate opaque struct */
	file = malloc(sizeof(*file));
	if (NULL == file)
		return NULL;

	/* Copy opened path so as not to depend on external data */
	file->path = str_copy(path, strlen(path));
	if (NULL == file->path)
		goto err_free_opaque;

	/* Allocate lines container to store lines */
	file->lines = vec_alloc(sizeof(struct line), FILE_LINES_CAP_STEP);
	if (NULL == file->lines)
		goto err_free_opaque_and_path;

	/* Initialize other fields */
	file->is_dirty = 0;
	return file;
err_free_opaque_and_path:
	free(file->path);
err_free_opaque:
	free(file);
	return NULL;
}

int
file_break_line(struct file *const file, const size_t idx, const size_t pos)
{
	int ret;
	size_t new_len;
	const char *new_chars;
	struct line new_line;
	struct line *line;

	/* Check current line not found */
	line = vec_get(file->lines, idx);
	if (NULL == line)
		return -1;

	/* Initialize new line */
	ret = line_init(&new_line);
	if (-1 == ret)
		return -1;

	/* Get new line length */
	new_len = vec_len(line->chars) - pos;

	/* Copy characters from broken line to new line if its length is not zero */
	if (new_len > 0) {
		/* Get start of part which we need to move to new line */
		new_chars = vec_get(line->chars, pos);
		if (NULL == new_chars)
			goto err_free;

		/* Append broken chars to new line */
		ret = line_append(&new_line, new_chars, new_len);
		if (-1 == ret)
			goto err_free;

		/* Cut broken line */
		ret = line_cut(line, pos);
		if (-1 == ret)
			goto err_free;
	}

	/* Insert new line */
	ret = vec_ins(file->lines, idx + 1, &new_line, 1);
	if (-1 == ret)
		goto err_free;

	/* Mark file as dirty because of new line */
	file->is_dirty = 1;
	return 0;
err_free:
	line_free(&new_line);
	return -1;
}

void
file_close(struct file *const file)
{
	file_free(file);
}

int
file_del_char(struct file *const file, const size_t idx, const size_t pos)
{
	int ret;
	struct line *line;

	/* Check line not found */
	line = vec_get(file->lines, idx);
	if (NULL == line)
		return -1;

	/* Remove character */
	ret = vec_remove(line->chars, pos, NULL);
	if (-1 == ret)
		return -1;

	/* Rerender updated line */
	ret = line_render(line);
	if (-1 == ret)
		return -1;

	/* Mark file as dirty */
	file->is_dirty = 1;
	return 0;
}

int
file_del_line(struct file *const file, const size_t idx)
{
	int ret;
	struct line line;

	/* Remember that file must contain at least one line */
	if (vec_len(file->lines) <= 1) {
		errno = ENOSYS;
		return -1;
	}

	/* Remove line using index */
	ret = vec_remove(file->lines, idx, &line);
	/* See removing function docs. line removed if argument is valid */
	if (errno != EINVAL)
		line_free(&line);
	if (-1 == ret)
		return -1;

	/* Mark file as dirty because of deleted line */
	file->is_dirty = 1;
	return 0;
}

static void
file_free(struct file *const file)
{
	struct line *lines;
	size_t len;

	/* Get lines and lines count */
	lines = vec_items(file->lines);
	len = vec_len(file->lines);

	/* Free lines  */
	while (len-- > 0)
		line_free(&lines[len]);
	vec_free(file->lines);

	/* Freeing the path since we cloned it earlier */
	free(file->path);
	/* Free allocated opaque struct */
	free(file);
}

int
file_ins_char(
	struct file *const file,
	const size_t idx,
	const size_t pos,
	const char ch
) {
	int ret;
	struct line *line;

	/* Check line not found */
	line = vec_get(file->lines, idx);
	if (NULL == line)
		return -1;

	/* Insert character to line */
	ret = vec_ins(line->chars, pos, &ch, 1);
	if (-1 == ret)
		return -1;

	/* Rerender line after character insertion */
	ret = line_render(line);
	if (-1 == ret)
		return -1;

	/* Mark file as dirty */
	file->is_dirty = 1;
	return 0;
}

int
file_ins_empty_line(struct file *const file, const size_t idx)
{
	int ret;
	struct line empty_line;

	/* Initialize empty line */
	ret = line_init(&empty_line);
	if (-1 == ret)
		return -1;

	/* Insert empty line */
	ret = vec_ins(file->lines, idx, &empty_line, 1);
	if (-1 == ret) {
		line_free(&empty_line);
		return -1;
	}

	/* Mark file as dirty */
	file->is_dirty = 1;
	return 0;
}

char
file_is_dirty(const struct file *const file)
{
	return file->is_dirty;
}

int
file_line(
	const struct file *const file,
	const size_t idx,
	struct pub_line *const line
) {
	const struct line *internal;

	/* Get internal line struct */
	internal = vec_get(file->lines, idx);
	if (NULL == line)
		return -1;

	/* Copy pointers and values to public line */
	line->chars = vec_items(internal->chars);
	line->len = vec_len(internal->chars);
	line->render = internal->render;
	line->render_len = internal->render_len;
	return 0;
}

size_t
file_lines_cnt(const struct file *const file)
{
	return vec_len(file->lines);
}

struct file*
file_open(const char *const path)
{
	int ret;
	FILE *inner_file;
	struct file *file;

	/* Check opaque struct allocation error */
	file = file_alloc(path);
	if (NULL == file)
		return NULL;

	/* Open file using path */
	inner_file = fopen(path, "r");
	if (NULL == inner_file)
		goto err_free_opaque;

	/* Read lines */
	ret = file_read(file, inner_file);
	if (-1 == ret)
		goto err_free_opaque_and_close_file;

	/* Close opened file */
	ret = fclose(inner_file);
	if (EOF == ret)
		goto err_free_opaque;

	/* Add empty line if there is no lines */
	if (vec_len(file->lines) == 0) {
		/* Insert empty line and reset dirty flag */
		ret = file_ins_empty_line(file, 0);
		if (-1 == ret)
			goto err_free_opaque;
		file->is_dirty = 0;
	}
	return file;
err_free_opaque_and_close_file:
	/* Errors checking is useless here */
	fclose(inner_file);
err_free_opaque:
	file_free(file);
	return NULL;
}

const char*
file_path(const struct file *const file)
{
	return file->path;
}

static int
file_read(struct file *const file, FILE *const inner)
{
	int ret;
	struct line line;

	/* Read lines until EOF */
	while (1) {
		/* Read new line */
		ret = line_read(&line, inner);
		if (1 != ret)
			return ret;

		/* Append readed line */
		ret = vec_append(file->lines, &line, 1);
		if (-1 == ret) {
			/* Free line which we can't append */
			line_free(&line);
			return -1;
		}
	}
}

size_t
file_save(struct file *const file, const char *const custom_path)
{
	int ret;
	FILE *inner;
	size_t len;
	const char *const path = NULL == custom_path ? file->path : custom_path;

	/* Try to open file */
	inner = fopen(path, "w");
	if (NULL == inner)
		return 0;

	/* Write lines to opened file */
	len = file_write(file, inner);
	if (0 == len)
		goto err_close;

	/* Flush written content */
	ret = fflush(inner);
	if (EOF == ret)
		goto err_close;

	/* Close file */
	ret = fclose(inner);
	if (EOF == ret)
		return 0;

	/* Remove dirty flag because file was saved */
	file->is_dirty = 0;
	return len;
err_close:
	/* Errors checking here is useless */
	fclose(inner);
	return 0;
}

size_t
file_save_to_spare_dir(struct file *const file, char *const path, size_t len)
{
	int ret;
	char date[15];
	const char *fname;
	const struct tm *local;
	time_t utc;

	/* Get filename */
	fname = basename(file->path);

	/* Get timestamp */
	utc = time(NULL);
	if ((time_t) - 1 == utc)
		return 0;

	/* Get local time from timestamp */
	local = localtime(&utc);
	if (NULL == local)
		return 0;

	/* Format local time to string */
	ret = strftime(date, sizeof(date), "%m-%d_%H-%M-%S", local);
	if (0 == ret)
		return 0;

	/* Build full spare path */
	ret = snprintf(path, len, "%s/%s_%s", cfg_spare_save_dir, fname, date);
	if (ret < 0 || (size_t)ret >= len)
		return 0;

	/* Save file using built path */
	return file_save(file, path);
}

int
file_search(
	struct file *const file,
	size_t *const idx,
	size_t *const pos,
	const char *const query,
	const enum dir dir
) {
	int ret;
	struct line *line;

	/* Try to get initial line */
	line = vec_get(file->lines, *idx);
	if (NULL == line)
		return -1;

	do {
		/* Try to search on line */
		ret = line_search(line, pos, query, dir);
		/* Return if result found or error happened */
		if (ret != 0)
			return ret;

		/* Break if the end of search reached */
		if (
			(DIR_BWD == dir && 0 == *idx)
			|| (DIR_FWD == dir && vec_len(file->lines) - 1 <= *idx)
		)
			break;

		/* Move to another line */
		*idx += DIR_FWD == dir ? 1 : -1;
		line = vec_get(file->lines, *idx);
		if (NULL == line)
			return -1;

		/* Choose position using direction */
		*pos = DIR_FWD == dir ? 0 : vec_len(line->chars);
	} while (*idx < vec_len(file->lines));
	return 0;
}

static size_t
file_write(const struct file *const file, FILE *const f)
{
	size_t i;
	size_t ret;
	size_t len = 0;
	const struct line *lines;

	/* Get lines */
	lines = vec_items(file->lines);

	/* Write lines and collect written length */
	for (i = 0; i < vec_len(file->lines); i++) {
		ret = line_write(&lines[i], f);
		if (0 == ret)
			return 0;
		len += ret;
	}
	return len;
}

static int
line_append(struct line *const line, const char *const chars, const size_t len)
{
	int ret;

	/* Copy chars to line */
	ret = vec_append(line->chars, chars, len);
	if (-1 == ret)
		return -1;

	/* Render line with new chars */
	ret = line_render(line);
	return ret;
}

static int
line_cut(struct line *const line, const size_t len)
{
	int ret;

	/* Update broken line's length */
	ret = vec_set_len(line->chars, len);
	if (-1 == ret)
		return -1;

	/* Shrink broken line's capacity if needed */
	ret = vec_shrink(line->chars, 0);
	if (-1 == ret)
		return -1;

	/* Render line with new length */
	ret = line_render(line);
	return ret;
}

void
line_free(struct line *const line)
{
	/* Free raw chars and render */
	vec_free(line->chars);
	free(line->render);
}

static int
line_init(struct line *const line)
{
	/* Allocate characters container */
	line->chars = vec_alloc(sizeof(char), LINE_CHARS_CAP_STEP);
	if (NULL == line->chars)
		return -1;

	/* Initialize render fields */
	line->render = NULL;
	line->render_len = 0;
	return 0;
}

static int
line_read(struct line *const line, FILE *const f)
{
	int ret;
	int ch;

	/* Initialize empty line to in which to read */
	ret = line_init(line);
	if (-1 == ret)
		return -1;

	/* Read characters */
	while (1) {
		/* Try to read character */
		ch = fgetc(f);
		if (ferror(f) != 0)
			goto err;

		/* First character is EOF. So there is no more lines */
		if (vec_len(line->chars) == 0 && EOF == ch) {
			/* Free unused line */
			line_free(line);
			return 0;
		}

		/* Check end of line reached */
		if ('\n' == ch)
			break;

		/* Append readed character */
		ret = vec_append(line->chars, &ch, 1);
		if (-1 == ret)
			goto err;
	}

	/* Shrink chars capacity to fit */
	ret = vec_shrink(line->chars, 1);
	if (-1 == ret)
		goto err;

	/* Render readed line */
	ret = line_render(line);
	if (-1 == ret)
		goto err;
	return 1;
err:
	line_free(line);
	return -1;
}

static int
line_render(struct line *const line)
{
	size_t i;
	size_t tabs = 0;
	const char *chars;

	/* Get characters */
	chars = vec_items(line->chars);

	/* Free old render */
	free(line->render);
	line->render = NULL;
	line->render_len = 0;

	/* No chars to render */
	if (vec_len(line->chars) == 0)
		return 0;

	/* Calculate tabs count */
	for (i = 0; i < vec_len(line->chars); i++) {
		if ('\t' == chars[i])
			tabs++;
	}

	/* Allocate render buffer */
	line->render = malloc(vec_len(line->chars) + (CFG_TAB_SIZE - 1) * tabs);
	if (NULL == line->render)
		return -1;

	/* Render chars */
	for (i = 0; i < vec_len(line->chars); i++) {
		if ('\t' == chars[i]) {
			/* Expand tab with spaces */
			line->render[line->render_len++] = ' ';
			while (line->render_len % CFG_TAB_SIZE != 0)
				line->render[line->render_len++] = ' ';
		} else {
			/* Render simple character */
			line->render[line->render_len++] = chars[i];
		}
	}
	return 0;
}

static int
line_search(
	struct line *const line,
	size_t *const idx,
	const char *const query,
	const enum dir dir
) {
	const char *fwd_start;
	const char *res = NULL;

	/* Validate accepted index */
	if (*idx > vec_len(line->chars)) {
		errno = EINVAL;
		return -1;
	}

	/* Check line is empty */
	if (vec_len(line->chars) == 0)
		return 0;

	/* Search backward or forward */
	switch (dir) {
	case DIR_BWD:
		res = strrstr(vec_items(line->chars), query, *idx);
		break;
	case DIR_FWD:
		/* Get start of searching and search */
		fwd_start = vec_get(line->chars, *idx);
		if (NULL == fwd_start)
			return -1;
		res = strnstr(fwd_start, query, vec_len(line->chars) - *idx);
		break;
	}

	/* Check if no results */
	if (NULL == res)
		return 0;
	/* Set result index and return success code */
	*idx = res - (char *)vec_items(line->chars);
	return 1;
}

static size_t
line_write(const struct line *const line, FILE *const f)
{
	int ret;
	size_t len;
	size_t written;

	/* Write line characters to the file */
	len = vec_len(line->chars);
	written = fwrite(vec_items(line->chars), sizeof(char), len, f);

	/* Check write error */
	if (written != len)
		return 0;

	/* Append newline to the end */
	ret = fputc('\n', f);
	if (EOF == ret)
		return 0;

	/* Return written length. Do not forget about newline character */
	return written + 1;
}
