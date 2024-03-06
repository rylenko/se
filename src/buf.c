#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buf.h"
#include "err.h"
#include "math.h"

#define REALLOC_STEP (4096)
#define FMT_STR_LEN (256)

/* Grows buffer capacity. */
static void buf_grow(Buf *buf, size_t by);

Buf
buf_alloc(void)
{
	return (Buf){ .data = NULL, .len = 0, .cap = 0 };
}

void
buf_flush(const Buf* buf, int fd)
{
	write(fd, buf->data, buf->len);
}

void
buf_free(Buf buf)
{
	free(buf.data);
}

static void
buf_grow(Buf *buf, size_t by)
{
	buf->cap += by;
	if (!(buf->data = realloc(buf->data, buf->cap))) {
		err("Failed to reallocate buffer with capacity %zu:", buf->cap);
	}
}

void
buf_write(Buf *buf, const char *part, size_t len)
{
	/* Check that we need to grow */
	size_t new_len = buf->len + len;
	if (new_len > buf->cap) {
		buf_grow(buf, MAX(new_len - buf->cap, REALLOC_STEP));
	}

	/* Append the part to buffer */
	memcpy(buf->data + buf->len, part, len);
	buf->len = new_len;
}

size_t
buf_writef(Buf *buf, const char *fmt, ...)
{
	va_list args;
	int len;
	char part[FMT_STR_LEN] = {0};

	/* Collect variadic arguments and print to new part of buffer */
	va_start(args, fmt);
	len = vsnprintf(part, FMT_STR_LEN, fmt, args);
	va_end(args);

	/* Write new formatted part */
	buf_write(buf, part, len);
	return len;
}
