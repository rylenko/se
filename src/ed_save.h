#ifndef _ED_SAVE_H
#define _ED_SAVE_H

/* Ed */
#include "ed.h"

/* Saves file to path. Saves to opened file if argument is `NULL`. */
void ed_save(Ed *, const char *);

/* Saves file to spare directory. Useful if no privileges. */
void ed_save_to_spare_dir(Ed *);

#endif /* _ED_SAVE_H */
