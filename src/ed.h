#ifndef _ED_H
#define _ED_H

/* Opaque struct with editor options. */
struct ed;

/*
 * Draws all window's content.
 *
 * Returns 0 on success and -1 on error.
 */
int ed_draw(struct ed *);

/*
 * Determines that we need to quit.
 */
char ed_need_to_quit(const struct ed *);

/*
 * Opens a file and binds editor to specified file descriptors. Do not forget
 * to quit it.
 *
 * Please quit the editor before printing, for example, error messages. This is
 * needed to disable raw mode and other settings properly.
 *
 * Returns pointer to opaque struct on success and `NULL` on error.
 */
struct ed *ed_open(const char *, int, int);

/*
 * Quits opened editor.
 *
 * Returns 0 on success and -1 on error.
 */
int ed_quit(struct ed *);

/*
 * Registers passed signal for future processing.
 */
void ed_reg_sig(struct ed *, int);

/*
 * Waits key press and processes it.
 *
 * Returns 0 on success and -1 on error.
 */
int ed_wait_and_proc_key(struct ed *);

#endif /* _ED_H */
