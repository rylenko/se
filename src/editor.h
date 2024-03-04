#ifndef _EDITOR_H
#define _EDITOR_H

/* Returns `1` if we need to quit the editor, otherwise `0`. */
char editor_need_to_quit(void);

/* Opens new file in the editor. */
void editor_open(const char *);

/* Refreshes the screen with editor's state. */
void editor_refresh_scr(void);

/* Waits and processes key presses from terminal's input. */
void editor_wait_and_proc_key_press(void);

#endif /* _EDITOR_H */
