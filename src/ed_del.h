#ifndef _ED_DEL_H
#define _ED_DEL_H

#include "ed.h"

/* Deletes current character. */
void ed_del(Ed *);

/* Deletes several rows from cursor's position. */
void ed_del_row(Ed *, size_t);

#endif /* _ED_DEL_H */
