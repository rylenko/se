#ifndef _ROW_H
#define _ROW_H

/* size_t */
#include <stddef.h>
/* FILE */
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

/* Dynamic array with `Row`s. */
typedef struct {
	Row *arr;
	size_t cap;
	size_t cnt;
} Rows;

/* Deletes character. */
void row_del(Row *, size_t);

/* Creates new empty row. */
Row row_empty(void);

/* Inserts character to row. Grows capacity if there is no space. */
void row_ins(Row *, size_t, char);

/* Breaks the row at index and position. */
void rows_break(Rows *, size_t, size_t);

/* Extends specified row with next row. */
void rows_extend_with_next(Rows *, size_t);

/* Remove row by its index. */
/* TODO: rename to `rows_remove` if undo is done. */
void rows_del(Rows *, size_t);

/* Frees allocated rows. */
void rows_free(Rows *);

/* Inserts new row at index. Grows capacity if there is no space. */
void rows_ins(Rows *, size_t, Row);

/* Creates new dynamic array with rows. */
Rows rows_new(void);

/* Reads rows from file without newline characters. */
void rows_read(Rows *, FILE *);

/* Writes rows to the file with newline characters. */
size_t rows_write(Rows *, FILE *);

#endif /* _ROW_H */
