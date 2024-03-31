#ifndef _WIN_DEL
#define _WIN_DEL

#include "win.h"

/*
Deletes the passed number of lines starting from the current one.

Returns zero on success and negative number if there is only one line which
cannot be deleted.
*/
int win_del_line(Win *, size_t);

#endif /* _WIN_DEL */
