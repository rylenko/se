#ifndef _ROW_H
#define _ROW_H

/* Row in the file. */
typedef struct {
	/* `NULL` if length is 0. */
	char *cont;
	size_t len;
} Row;

/* Dynamic array with `Row`s. */
typedef struct {
	Row *arr;
	size_t cap;
	size_t cnt;
} Rows;

/* Creates new empty row. */
Row row_empty(void);

/* Creates new dynamic array with rows. */
Rows rows_alloc(void);

/* Frees allocated rows. */
void rows_free(Rows *);

/* Inserts new row at index. */
void rows_ins(Rows *, size_t, Row);

/* Reads rows from file without newline character. */
void rows_read(Rows *, FILE *);

#endif /* _ROW_H */
