#ifndef _ROW_H
#define _ROW_H

/* Row in the file. */
typedef struct {
	/* `NULL` if length is 0. */
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
void row_del(Row *, const size_t);

/* Creates new empty row. */
Row row_empty(void);

/* Inserts character to row. */
void row_ins(Row *, const size_t, const char);

/* Remove row by its index. */
/* TODO: rename to `rows_remove` if undo is done. */
void rows_del(Rows *, const size_t);

/* Frees allocated rows. */
void rows_free(Rows *);

/* Inserts new row at index. */
void rows_ins(Rows *, const size_t, Row);

/* Creates new dynamic array with rows. */
Rows rows_new(void);

/* Reads rows from file without newline characters. */
void rows_read(Rows *, FILE *);

/* Writes rows to the file with newline characters. */
size_t rows_write(Rows *, FILE *);

#endif /* _ROW_H */
