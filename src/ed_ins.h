#ifndef _ED_INS_H
#define _ED_INS_H

/* Inserts a character. */
void ed_ins(Ed *, char);

/* Breaks the row into two rows. */
void ed_ins_break(Ed *);

/* Inserts new row below the cursor. */
void ed_ins_row_below(Ed *, size_t);

/* Inserts new row on top of the cursor. */
void ed_ins_row_top(Ed *, size_t);

#endif /* _ED_INS_H */
