#ifndef _WIN_MV
#define _WIN_MV

#include "win.h"

/* Move down several times. */
void win_mv_down(Win *, size_t);

/* Move left several times. */
void win_mv_left(Win *, size_t);

/* Move right several times. */
void win_mv_right(Win *, size_t);

/* Moves to begin of first line. */
void win_mv_to_begin_of_file(Win *);

/* Moves to begin of current line. */
void win_mv_to_begin_of_line(Win *);

/* Moves to begin of last line. */
void win_mv_to_end_of_file(Win *);

/* Moves to begin of current line. */
void win_mv_to_end_of_line(Win *);

/* Moves up several times. */
void win_mv_up(Win *, size_t);

#endif /* _WIN_MV */
