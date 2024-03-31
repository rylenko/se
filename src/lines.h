#ifndef _LINES_H
#define _LINES_H

#include <stdio.h>
#include "line.h"

/* Liness of the file. */
typedef struct {
	Line *arr; /* Dynamic array with liness */
	size_t cnt; /* Count of lines */
	size_t cap; /* Reserved capacity for dynamic array */
} Lines;

/* TODO: do not forget to rerender after absorb */
/*
Extends specified line with next line, then frees next line.

Does not update the render so you can do it yourself after several operations.
*/
void lines_absorb_next(Lines *, size_t);

/* Finds the line by index and breaks it at specified position. */
void lines_break(Lines *, size_t, size_t);

/* Deletes line by index. */
void lines_del(Lines *, size_t);

/* Frees the lines container. */
void lines_free(Lines *);

/* Gets the line by its index. */
Line *lines_get(const Lines *, size_t);

/* Initializes lines with default values. Do not forget to free it. */
void lines_init(Lines *);

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
