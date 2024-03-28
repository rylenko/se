#ifndef _CUR_H
#define _CUR_H

typedef struct {
	unsigned short col; /* Row's number, starts from zero */
	unsigned short row; /* Column's number, starts from zero */
} Cur;

/* Initializes the cursor with zeros. */
void cur_init(Cur *);

#endif /* _CUR_H */
