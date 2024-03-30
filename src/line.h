#ifndef _LINE_H
#define _LINE_H

#include <stdio.h>

typedef struct {
	char *cont; /* Raw content of the line */
	size_t len; /* Length of content */
	size_t cap; /* Capacity of content's buffer */
	char *render; /* Rendered content: tabs expansios, etc. */
	size_t render_len; /* Rendered content length */
} Line;

/*
Deletes character from the line.

Does not update the render so you can do it yourself after several operations.
*/
void line_del(Line *, size_t);

/*
Extends line with another line's content.

Does not update the render so you can do it yourself after several operations.
*/
void line_extend(Line *, const Line *);

/* Frees and zeroizes line's memory. */
void line_free(Line *);

/*
Grows line's capacity if there is no space for new characters of passed length.
*/
void line_grow_if_needed(Line *, size_t);

/* Initializes line with default values. Do not forget to free it. */
void line_init(Line *);

/*
Inserts character to line at index.

Does not update the render so you can do it yourself after several operations.
*/
void line_ins(Line *, size_t, char);

/*
Reads a line from a file without `'\n'`.

Returns the passed line on success. Returns `NULL` if `EOF` is reached.
*/
Line* line_read(Line *, FILE *);

/*
Renders line's content how it should look on the window. For example, unfolds
tabs.
*/
void line_render(Line *);

/*
Shrinks line's capacity.

If the line is empty and memory is allocated, the function will free it.

If flag is `1`, then function reallocates line with capacity equal to actual
size. Otherwise it will reallocate memory if a lot of capacity is not used.
*/
void line_shrink(Line *, char);

/*
Writes a line to the file with `'\n'` at the end.

Returns written bytes count.
*/
size_t line_write(Line *, FILE *);

#endif /* _LINE_H */
