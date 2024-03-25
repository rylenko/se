#ifndef _BUF_H
#define _BUF_H

#include <stddef.h>

/*
During redrawing content may flicker because `printf` buffers the output but
flushes it to the terminal after receiving '\n'.

This buffer is needed to write strings into one large buffer and print them on
the window in one call.
*/
typedef struct {
	char *data; /* Dynamic array with buffer data */
	size_t len; /* Length of data */
	size_t cap; /* Current capacity of dynamic array with data */
} Buf;

/* Initializes buffer with default values. Do not forget to free it. */
void buf_init(Buf *);

/*
Flushes the buffer to file by its descritor using single `write` call.

After flush frees and zeroizes the buffer to continue usage from scratch.
*/
void buf_flush(Buf *, int);

/* Frees the buffer. */
void buf_free(Buf *);

/* Appends a string with specified length to the buffer. */
size_t buf_write(Buf *, const char *, size_t);

/* Appends a formatted string to the buffer .*/
size_t buf_writef(Buf *, const char *, ...);

#endif /* _BUF_H */
