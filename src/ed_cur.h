#ifndef _ED_CUR_H
#define _ED_CUR_H

/* Ed */
#include "ed.h"

/*
Fixes cursor's coordinates.

Useful when resizing the window or moving to a another row.
*/
void ed_cur_fix(Ed *);

#endif /* _ED_CUR_H */
