#ifndef _ROWS_H
#define _ROWS_H

#include <stdio.h>
#include "row.h"

/* Rows of the file. */
typedef struct {
	Row *arr; /* Dynamic array with rows */
	size_t cnt; /* Count of rows */
	size_t cap; /* Reserved capacity for dynamic array */
} Rows;

/* TODO: do not forget to rerender after absorb */
/*
Extends specified row with next row, then frees next row.

Does not update the render so you can do it yourself after several operations.
*/
void rows_absorb_next(Rows *, size_t);

/* Find the row by index and breaks it at specified position. */
void rows_break(Rows *, size_t, size_t);

/* Deletes row by index. */
void rows_del(Rows *, size_t);

/* Frees the rows container. */
void rows_free(Rows *);

/* Initializes rows container with default values. Do not forget to free it. */
void rows_init(Rows *);

/* Inserts new row at index. */
void rows_ins(Rows *, size_t, Row);

/* Reads rows from the file. */
void rows_read(Rows *, FILE *);

/*
Writes rows to the file.

Returns written bytes count.
*/
size_t rows_write(const Rows *, FILE *);

#endif /* _ROWS_H */
