#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buf.h"
#include "err.h"

#define FMT_STR_LEN (256)

Buf
buf_alloc(void)
{
	return (Buf){ .data = NULL, .len = 0 };
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

void
buf_write(Buf *buf, const char *part, size_t len)
{
	/* Reallocate with new length */
	size_t new_len = buf->len + len;
	char *new_data = realloc(buf->data, new_len);
	if (!new_data)
		err("Failed to reallocate buffer data with length %zu:", new_len);

	/* Copy the part to buffer */
	memcpy(new_data + buf->len, part, len);

	/* Update buffer with reallocated data and new length */
	buf->data = new_data;
	buf->len = new_len;
}

void
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
}
