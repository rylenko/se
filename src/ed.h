#ifndef _ED_H
#define _ED_H

/* Deinitializes editor. */
void ed_deinit(void);

/* Initializes editor. */
void ed_init(int, int);

/* Returns `1` if we need to quit the editor, otherwise `0`. */
char ed_need_to_quit(void);

/* Opens new file in the editor. */
void ed_open(const char *);

/* Refreshes the window with editor's state. */
void ed_refr_win(void);

/* Waits and processes key presses from terminal's input. */
void ed_wait_and_proc_key(void);

#endif /* _ED_H */
