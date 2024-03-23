#ifndef _ED_H
#define _ED_H

#include "file.h"
#include "mode.h"
#include "win.h"

enum {
	ED_MSG_ARR_LEN = 64, /* Capacity of message buffer */
};

/* Editor options. */
typedef struct {
	File file; /* Info about opened file. This is what the user edits */
	Win win; /* Info about view. This is what the user sees */
	Mode mode; /* Input mode */
	char msg[ED_MSG_ARR_LEN]; /* Message for the user */
	size_t num_input; /* Number input. Maximum if not set */
	unsigned char quit_presses_rem; /* Greater than 1 if file is dirty */
} Ed;

#endif /* _ED_H */
