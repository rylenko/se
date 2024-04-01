#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buf.h"
#include "err_alloc.h"
#include "math.h"

enum {
	BUF_REALLOC_STEP = 4096, /* Realloc step if there is no space for new data */
	BUF_FMTED_STR_MAX_LEN = 255, /* Maximum length of formatted string */
};

/*
During redrawing content may flicker because `printf` buffers the output but
flushes it to the terminal after receiving '\n'.

This buffer is needed to write strings into one large buffer and print them on
the window in one call.
*/
struct Buf {
	char *data; /* Dynamic array with buffer data */
	size_t len; /* Length of data */
	size_t cap; /* Current capacity of dynamic array with data */
};

/* Reallocates buffer with new capacity. */
static void buf_realloc(Buf *, size_t);

Buf*
buf_alloc(void)
{
	return err_calloc(1, sizeof(Buf));
}

void
buf_flush(Buf *const buf, const int fd)
{
	/* Write buffer's data to file by its descriptor */
	if (write(fd, buf->data, buf->len) < 0)
		err(EXIT_FAILURE, "Failed to flush the buffer with length %zu", buf->len);
	/* Refresh length to continue from scratch */
	buf->len = 0;
}

void
buf_free(Buf *const buf)
{
	/* Deallocate internal data and buffer */
	free(buf->data);
	free(buf);
}

static void
buf_realloc(Buf *const buf, const size_t new_cap)
{
	/* Reallocate and update capacity */
	buf->data = err_realloc(buf->data, new_cap);
	buf->cap = new_cap;
}

size_t
buf_write(Buf *const buf, const char *const str, const size_t len)
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
buf_writef(Buf *const buf, const char *const fmt, ...)
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
