#ifndef _ROW_H
#define _ROW_H

/* Row in the file. */
typedef struct {
	char *cont;
	size_t len;
} Row;

/* Dynamic array with `Row`s. */
typedef struct {
	Row *arr;
	size_t cap;
	size_t cnt;
} Rows;

/* Creates new dynamic array with `Row`s. */
Rows rows_alloc(void);

/* Frees allocated rows. */
void rows_free(Rows *);

/* Reads rows from file without newline character. */
void rows_read(Rows *, FILE *);

#endif /* _ROW_H */
