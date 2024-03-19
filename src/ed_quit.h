#ifndef _ED_QUIT_H
#define _ED_QUIT_H

/* Determines that we need to quit immediately beacuse of freed memory.. */
char ed_quit_done(const Ed *);

/*
Tries to quit the editor.

It may not quit because we have a counter for dirty files.
*/
void ed_quit_try(Ed *);

#endif /* _ED_QUIT_H */
