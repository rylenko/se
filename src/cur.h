#ifndef _CUR_H
#define _CUR_H

typedef struct {
	unsigned short x; /* x coordinate, starts from zero */
	unsigned short y; /* y coordinate, starts from zero */
} Cur;

/* Initializes the cursor. */
void cur_init(Cur *);

#endif /* _CUR_H */
