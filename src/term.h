#ifndef _TERM_H
#define _TERM_H

#include <stddef.h>
#include <sys/ioctl.h>
#include "buf.h"

/* Deinitializes terminal and disables raw mode. */
void term_deinit(void);

/* Flushes the buffer to terminal. */
void term_flush(Buf *);

/* Gets terminal's window size. */
void term_get_win_size(struct winsize *);

/*
Initializes terminal with input file descriptor and output file descriptor and
enables raw mode.

Do not forget to deinitialize it.
*/
void term_init(int, int);

/*
Waits for a key press.

Key press sometimes consists of several characters. Therefore, the pressed
key's characters is written to the passed buffer up to the passed length.

Returns readed characters count.
*/
size_t term_wait_key(char *, size_t);

#endif /* _TERM_H */
