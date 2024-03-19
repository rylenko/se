#ifndef _ROW_H
#define _ROW_H

#include <stddef.h>
#include <stdio.h>

/*
Row in the file.

Length is greater than zero, otherwise content is `NULL`.
*/
typedef struct {
	size_t cap;
	char *cont;
	size_t len;
} Row;

/* Deletes character. */
void row_del(Row *, size_t);

/* Creates new empty row. */
Row row_empty(void);

void row_free(Row *);

/* Extends row with another row. */
void row_extend(Row *, const Row *);

/* Inserts character to row. Grows capacity if there is no space. */
void row_ins(Row *, size_t, char);

/* Shrinks row if needed. */
void row_shrink_if_needed(Row *);

/*
Reads row from file.

Returns pointer to accepted row on success and `NULL` on `EOF`.
*/
Row *row_read(Row *, FILE *);

/* Writes row to file. */
size_t row_write(Row *, FILE *);

#endif /* _ROW_H */
