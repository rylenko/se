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

/* Deinitializes window's from terminal. */
void win_deinit(Win *);

/*
Initializes and connects window to terminal using passed file descriptors.

Do not forget to deinitialize it.
*/
void win_init(Win *, int, int);

#endif /* WIN_H */
