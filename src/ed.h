#ifndef _ED_H
#define _ED_H

#include "file.h"
#include "mode.h"
#include "win.h"

enum {
	ED_MSG_ARR_LEN = 64, /* Capacity of message buffer */
};

/*
Editor options.

To edit, you need to connect the editor to the terminal and then open the file.
*/
typedef struct {
	File file; /* Opened file's info. This is what the user edits */
	Win win; /* Info about terminal's view. This is what the user sees */
	Mode mode; /* Input mode */
	char msg[ED_MSG_ARR_LEN]; /* Message for the user */
	size_t num_input; /* Number input. Maximum if not set */
	unsigned char quit_presses_rem; /* Greater than 1 if file is dirty */
} Ed;

/* Closes opened editor. */
void ed_close(Ed *);

/* Writes digit to the number input. Argument must be a digit. */
void ed_input_num(Ed *, unsigned char);

/* Sets formatted message to the user. */
void ed_set_msg(Ed *, const char *, ...);

/* Marks the file as dirty and sets several presses to quit without saving. */
void ed_on_file_ch(Ed *);

/*
Opens a file and binds editor to specified file descriptors.

Do not forget to close the editor.
*/
void ed_open(Ed *, const char *, int, int);

#endif /* _ED_H */
