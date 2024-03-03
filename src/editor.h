#ifndef _EDITOR_H
#define _EDITOR_H

#include "term.h"

typedef struct {
	const char *path;
} Editor;

/* Creates new editor from terminal struct. */
Editor editor_new(const char *);

/* Refreshes the screen with editor's state. */
void editor_refresh_scr(const Editor *);

#endif /* _EDITOR_H */
