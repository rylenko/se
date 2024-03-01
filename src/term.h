#ifndef _TERM_H
#define _TERM_H

#include <termios.h>
#include "res.h"

/* Disables raw mode. Accepts original parameters. */
Res term_disable_raw_mode(const struct termios*);

/* Enables raw mode. Accepts a pointer to save the original parameters. */
Res term_enable_raw_mode(struct termios*);

#endif /* _TERM_H */
