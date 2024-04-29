#ifndef _ED_H
#define _ED_H

/* Opaque struct with editor options. */
typedef struct ed Ed;

/*
Draws all window's content.

Returns 0 on success and -1 on error.
*/
int ed_draw(Ed *);

/* Handles signal for editor. */
void ed_handle_signal(Ed *, int);

/* Determines that we need to quit. */
char ed_need_to_quit(const Ed *);

/*
Opens a file and binds editor to specified file descriptors. Do not forget to
quit it.

Returns pointer to opaque struct on success and `NULL` on error.
*/
Ed *ed_open(const char *, int, int);

/*
Quits opened editor.

Returns 0 on success and -1 on error.
*/
int ed_quit(Ed *);

/*
Waits key press and processes it.

Returns 0 on success and -1 on error.
*/
int ed_wait_and_proc_key(Ed *);

#endif /* _ED_H */
