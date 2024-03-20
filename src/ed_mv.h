#ifndef _ED_MV_H
#define _ED_MV_H

#include "ed.h"

/* Move to begin of file. */
void ed_mv_begin_of_f(Ed *);

/* Move to begin of row. */
void ed_mv_begin_of_row(Ed *);

/* Move cursor down. */
void ed_mv_down(Ed *, size_t);

/* Move to end of file. */
void ed_mv_end_of_f(Ed *);

/* Move to end of row. */
void ed_mv_end_of_row(Ed *);

/* Move cursor left. */
void ed_mv_left(Ed *, size_t);

/* Move cursor to next word if exists. */
void ed_mv_next_word(Ed *, size_t);

/* Move cursor to previous word if exists. */
void ed_mv_prev_word(Ed *, size_t);

/* Move cursor right. */
void ed_mv_right(Ed *, size_t);

/* Move cursor up several times. */
void ed_mv_up(Ed *, size_t);

#endif /* _ED_MV_H */
