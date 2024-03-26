#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buf.h"
#include "math.h"

enum {
	BUF_REALLOC_STEP = 4096, /* Realloc step if there is no space for new data */
	BUF_FMTED_STR_MAX_LEN = 255, /* Maximum length of formatted string */
};

/* Reallocates buffer with new capacity. */
static void buf_realloc(Buf *const buf, const size_t new_cap);

void
buf_init(Buf *const buf)
{
	memset(buf, 0, sizeof(*buf));
}

void
buf_flush(Buf *const buf, const int fd)
{
	/* Write buffer's data to file by its descriptor */
	if (write(fd, buf->data, buf->len) < 0)
		err(EXIT_FAILURE, "Failed to flush the buffer with length %zu", buf->len);

	/* Refresh buffer to continue from scratch */
	buf_free(buf);
	memset(buf, 0, sizeof(*buf));
}

void
buf_free(Buf *const buf)
{
	/* Free allocated data */
	free(buf->data);
}

static void
buf_realloc(Buf *const buf, const size_t new_cap)
{
	if (NULL == (buf->data = realloc(buf->data, new_cap)))
		err(EXIT_FAILURE, "Failed to reallocate buffer with capacity %zu", new_cap);
	/* Set new capacity */
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
	char str[BUF_FMTED_STR_MAX_LEN + 1] = {0};

	/* Collect variadic arguments and print formatted string to array */
	va_start(args, fmt);
	len = vsnprintf(str, sizeof(str), fmt, args);
	va_end(args);

	/* Write formatted string to buffer */
	return buf_write(buf, str, len);
}
