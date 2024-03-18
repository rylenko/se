#ifndef _CUR_H
#define _CUR_H

/* Cursor struct with position on the screen. */
typedef struct {
	unsigned short x;
	unsigned short y;
} Cur;

/* Hides the cursor. */
void cur_hide(Buf *);

/* Creates new cursor position. */
Cur cur_new(unsigned short, unsigned short);

/* Shows the cursor. */
void cur_show(Buf *);

/* Writes cursor position to buffer. */
void cur_write(Cur, Buf *);

#endif /* _CUR_H */
