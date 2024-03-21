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
	char *render;
	size_t render_len;
} Row;

/* Deletes character. Does not update render. */
void row_del(Row *, size_t);

/* Creates new empty row. */
Row row_empty(void);

/* Frees row's content. */
void row_free(Row *);

/* Extends row with another row. Does not update render. */
void row_extend(Row *, const Row *);

/* Inserts character to row. Does not update render. */
void row_ins(Row *, size_t, char);

/* Shrinks row if needed. */
void row_shrink_if_needed(Row *);

/*
Reads row from file.

Returns pointer to accepted row on success and `NULL` on `EOF`.
*/
Row *row_read(Row *, FILE *);

/* Updates rendered version of row's content. */
void row_upd_render(Row *);

/* Writes row to file. */
size_t row_write(Row *, FILE *);

#endif /* _ROW_H */
