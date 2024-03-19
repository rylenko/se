#ifndef _CUR_H
#define _CUR_H

#include "buf.h"

/* Cursor struct with position on the window. */
typedef struct {
	unsigned short x;
	unsigned short y;
} Cur;

/* Draws cursor position to buffer. */
void cur_draw(Cur, Buf *);

/* Hides the cursor. */
void cur_hide(Buf *);

/* Creates new cursor position. */
Cur cur_new(unsigned short, unsigned short);

/* Shows the cursor. */
void cur_show(Buf *);

#endif /* _CUR_H */
