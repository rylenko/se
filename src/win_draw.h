#ifndef _WIN_DRAW_H
#define _WIN_DRAW_H

#include "buf.h"
#include "win.h"

/* Draws cursor. */
void win_draw_cur(const Win *, Buf *);

/* Draws window rows. */
void win_draw_lines(const Win *, Buf *);

#endif /* _WIN_DRAW_H */
