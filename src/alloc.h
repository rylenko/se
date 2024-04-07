#ifndef _ALLOC_H
#define _ALLOC_H

#include <stddef.h>

/* Like `calloc`, but exits with formatted error message on fail. */
void *calloc_err(size_t, size_t);

/* Like `malloc`, but exits with formatted error message on fail. */
void *malloc_err(size_t);

/* Like `realloc`, but exits with formatted error message on fail. */
void *realloc_err(void *, size_t);

#endif /* _ALLOC_H */
