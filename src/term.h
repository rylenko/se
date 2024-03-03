#ifndef _TERM_H
#define _TERM_H

#include <termios.h>

/* Disables raw mode. Accepts original parameters. */
void term_disable_raw_mode(int fd, const struct termios *);

/* Enables raw mode. Accepts a pointer to save the original parameters. */
void term_enable_raw_mode(int fd, struct termios *);

#endif /* _TERM_H */
