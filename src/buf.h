#ifndef _BUF_H
#define _BUF_H

#include <stddef.h>

/* Alias for opaque struct of buffer. */
typedef struct Buf Buf;

/* Allocatees empty buffer. Do not forget to free it. */
Buf *buf_alloc(void);

/*
Flushes the buffer to terminal using single system call.

After flush frees and zeroizes the buffer to continue usage from scratch.
*/
void buf_flush(Buf *);

/* Frees the buffer. */
void buf_free(Buf *);

/* Appends a string with specified length to the buffer. */
size_t buf_write(Buf *, const char *, size_t);

/* Appends a formatted string to the buffer .*/
size_t buf_writef(Buf *, const char *, ...);

#endif /* _BUF_H */
