#ifndef _ED_TERM_H
#define _ED_TERM_H

#include "ed.h"

/* Disconnects editor with terminal. */
void ed_term_deinit(void);

/* Connectes editor with terminal. */
void ed_term_init(Ed *, int, int);

#endif /* _ED_TERM_H */
