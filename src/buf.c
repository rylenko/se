#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "alloc.h"
#include "buf.h"
#include "math.h"
#include "term.h"

enum {
	BUF_REALLOC_STEP = 4096, /* Realloc step if there is no space for new data */
	BUF_FMTED_STR_MAX_LEN = 255, /* Maximum length of formatted string */
};

/*
If write frequently, the following problems may occur:
1. Content may flicker because everything reaches the terminal in pieces.
2. Lots of write system calls used.

Data can be written to this buffer. Then data can be written to the terminal
in one piece in one system call.
*/
struct Buf {
	char *data; /* Dynamic array with buffer data */
	size_t len; /* Length of data */
	size_t cap; /* Current capacity of dynamic array with data */
};

/* Reallocates buffer with new capacity. */
static void buf_realloc(struct Buf *, size_t);

struct Buf*
buf_alloc(void)
{
	return calloc_err(1, sizeof(struct Buf));
}

void
buf_flush(struct Buf *const buf)
{
	/* Write buffer's data to terminal */
	term_write(buf->data, buf->len);
	/* Refresh length to continue from scratch */
	buf->len = 0;
}

void
buf_free(struct Buf *const buf)
{
	/* Deallocate internal data and buffer */
	free(buf->data);
	free(buf);
}

static void
buf_realloc(struct Buf *const buf, const size_t new_cap)
{
	/* Reallocate and update capacity */
	buf->data = realloc_err(buf->data, new_cap);
	buf->cap = new_cap;
}

size_t
buf_write(struct Buf *const buf, const char *const str, const size_t len)
{
	/* Realloce if there is no space for new data */
	if (buf->len + len > buf->cap)
		buf_realloc(buf, MAX(buf->cap + BUF_REALLOC_STEP, buf->len + len));

	/* Copy new string to buffer */
	memcpy(&buf->data[buf->len], str, len);
	/* Update buffer length */
	buf->len += len;
	return len;
}

size_t
buf_writef(struct Buf *const buf, const char *const fmt, ...)
{
	va_list args;
	int len;
	char str[BUF_FMTED_STR_MAX_LEN + 1];

	/* Collect variadic arguments and print formatted string to array */
	va_start(args, fmt);
	len = vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	/* Write formatted string to buffer */
	return buf_write(buf, str, len);
}
