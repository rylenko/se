#ifndef _BUF_H
#define _BUF_H

#include <stdlib.h>

/*
During redrawing content may flicker because `printf` buffers the output but
flushes it to the terminal after receiving '\n'.

This buffer is needed to write strings into one large buffer and print them on
the window in one call.
*/
typedef struct {
	size_t cap;
	char *data;
	size_t len;
} Buf;

/* Creates new buffer. Do not forget to `buf_free` it. */
Buf buf_alloc(void);

/* Writes buffer to stdout using one `write` call. */
void buf_flush(const Buf *, int);

/* Frees the buffer. */
void buf_free(Buf *);

/* Appends a string to a buffer. */
size_t buf_write(Buf *, const char *, size_t);

/* Appends a formatted string to a buffer. */
size_t buf_writef(Buf *, const char *, ...);

#endif /* _BUF_H */
