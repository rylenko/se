#ifndef _TERM_H
#define _TERM_H

#include <sys/ioctl.h>
#include "buf.h"

/* Disables raw mode. Accepts original parameters. */
void term_disable_raw_mode(void);

/* Enables raw mode. Accepts a pointer to save the original parameters. */
void term_enable_raw_mode(void);

/* Gets wingow size: rows and colums count. */
void term_get_win_size(struct winsize *);

/* Places the cursor at the beginning. */
void term_go_home(Buf *buf);

/* Flushes buffer to terminal */
void term_flush(const Buf *buf);

/* Hides the cursor on screen. */
void term_hide_cur(Buf *buf);

/* Creates new terminal controller. */
void term_init(const int, const int);

/* Shows the cursor on screen. */
void term_show_cur(Buf *buf);

#endif /* _TERM_H */
