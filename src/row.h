#ifndef _ROW_H
#define _ROW_H

#include <stdio.h>

typedef struct {
	char *cont; /* Raw content of the row */
	size_t len; /* Length of content */
	size_t cap; /* Capacity of content's buffer */
	char *render; /* Rendered content: tabs expansios, etc. */
	size_t render_len; /* Rendered content length */
} Row;

/*
Deletes character from the row.

Does not update the render so you can do it yourself after several operations.
*/
void row_del(Row *const, size_t);

/*
Extends row with another row's content.

Does not update the render so you can do it yourself after several operations.
*/
void row_extend(Row *, const Row *);

/* Frees and zeroizes row's memory. */
void row_free(Row *);

/*
Grows row's capacity if there is no space for new characters of passed length.
*/
void row_grow_if_needed(Row *, size_t);

/* Initializes row with default values. Do not forget to free it. */
void row_init(Row *);

/*
Inserts character to row at index.

Does not update the render so you can do it yourself after several operations.
*/
void row_ins(Row *, size_t, char);

/*
Reads a row from a file without `'\n'`.

Returns the passed row on success. Returns `NULL` if `EOF` is reached.
*/
Row* row_read(Row *, FILE *);

/*
Renders row's content how it should look on the window. For example, unfolds
tabs.
*/
void row_render(Row *);

/*
Shrinks row's capacity.

If the row is empty and memory is allocated, the function will free it.

If flag is `1`, then function reallocates row with capacity equal to actual
size. Otherwise it will reallocate memory if a lot of capacity is not used.
*/
void row_shrink(Row *, char);

/*
Writes a row to the file with `'\n'` at the end.

Returns written bytes count.
*/
size_t row_write(Row *, FILE *);

#endif /* _ROW_H */
