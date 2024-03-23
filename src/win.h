#ifndef _WIN_H
#define _WIN_H

#include <sys/ioctl.h>
#include "pos.h"

/*
Information about what the user sees on the screen.

The position in the window may differ from the position in the file, for
example due to tabs.
*/
typedef struct {
	Pos pos; /* Position in the window */
	struct winsize size; /* Window size */
} Win;

#endif /* WIN_H */
