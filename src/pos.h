#ifndef _POS_H
#define _POS_H

#include "cur.h"

typedef struct {
	struct {
		size_t x;
		size_t y;
	} offset; /* Content offset */
	Cur cur; /* Cursor on observed content */
} Pos;

/* Initializes the position. */
void pos_init(Pos *);

#endif /* _POS_H */
