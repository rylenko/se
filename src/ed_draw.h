#ifndef _ED_DRAW_H
#define _ED_DRAW_H

#include "ed.h"

/* Draws all window's content. */
void ed_draw(Ed *);

/* Handles drawing signals. */
void ed_draw_handle_signal(Ed *, int);

#endif /* _ED_DRAW_H */
