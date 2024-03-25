#ifndef _CUR_H
#define _CUR_H

typedef struct {
	unsigned short x; /* X coordinate in the window, starts from zero */
	unsigned short y; /* Y coordinate in the window, starts from zero */
} Cur;

/* Initializes the cursor with zeros. */
void cur_init(Cur *);

#endif /* _CUR_H */
