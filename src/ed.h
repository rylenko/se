#ifndef _ED_H
#define _ED_H

/* Opaque struct with editor options. */
typedef struct Ed Ed;

/* Draws all window's content. */
void ed_draw(Ed *);

/* Handles signal for editor. */
void ed_handle_signal(Ed *, int);

/* Determines that we need to quit. */
char ed_need_to_quit(const Ed *);

/*
Opens a file and binds editor to specified file descriptors.

Do not forget to close the editor.
*/
Ed *ed_open(const char *, int, int);

/* Quits opened editor. */
void ed_quit(Ed *);

/* Waits key press and processes it. */
void ed_wait_and_proc_key(Ed *);

#endif /* _ED_H */
