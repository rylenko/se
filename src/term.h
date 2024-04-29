#ifndef _TERM_H
#define _TERM_H

#include <unistd.h>
#include <sys/ioctl.h>

/*
Deinitializes initialized terminal.

Returns 0 on success and -1 on error.
*/
int term_deinit(void);

/*
Gets terminal's window size.

Returns 0 on success and -1 on error.
*/
int term_get_win_size(struct winsize *);

/*
Initializes terminal with input file descriptor and output file descriptor and
enables raw mode. Do not forget to deinitialize it.

Returns 0 on success and -1 on error.
*/
int term_init(int, int);

/*
Waits for a key press.

Key press sometimes consists of several characters. Therefore, the pressed
key's characters is written to the passed buffer up to the passed length.

Returns readed characters count on success and 0 on error.
*/
size_t term_wait_key(char *, size_t);

/*
Writes passed data to the terminal in one system call.

Returns 0 on success and -1 on error.
*/
ssize_t term_write(const char *, size_t);

#endif /* _TERM_H */
