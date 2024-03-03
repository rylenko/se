#ifndef _EDITOR_H
#define _EDITOR_H

/* Initializes the editor. Use it before opening the file. */
void editor_init(void);

/* Opens new file in the editor. */
void editor_open(const char *);

/* Refreshes the screen with editor's state. */
void editor_refresh_scr(void);

#endif /* _EDITOR_H */
