#ifndef _LINES_H
#define _LINES_H

#include <stdio.h>
#include "line.h"

/* Opaque struct to represent file lines. */
typedef struct Lines Lines;

/* TODO: do not forget to rerender after absorb */
/*
Extends specified line with next line, then frees next line.

Does not update the render so you can do it yourself after several operations.
*/
void lines_absorb_next(Lines *, size_t);

/* Allocates lines container. Do not forget to free it. */
Lines *lines_alloc(void);

/* Finds the line by index and breaks it at specified position. */
void lines_break(Lines *, size_t, size_t);

/* Returns lines count. */
size_t lines_cnt(const Lines *);

/* Deletes line by index. */
void lines_del(Lines *, size_t);

/* Frees the lines container. */
void lines_free(Lines *);

/* Gets the line by its index. */
Line *lines_get(const Lines *, size_t);

/* Inserts new line at index. */
void lines_ins(Lines *, size_t, Line);

/* Reads lines from the file. */
void lines_read(Lines *, FILE *);

/*
Writes lines to the file.

Returns written bytes count.
*/
size_t lines_write(const Lines *, FILE *);

#endif /* _LINES_H */
