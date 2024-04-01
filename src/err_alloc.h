#ifndef _ERR_ALLOC_H
#define _ERR_ALLOC_H

/* Like `calloc`, but exits with formatted error message on fail. */
void *err_calloc(size_t, size_t);

/* Like `malloc`, but exits with formatted error message on fail. */
void *err_malloc(size_t);

/* Like `realloc`, but exits with formatted error message on fail. */
void *err_realloc(void *, size_t);

#endif /* _ERR_ALLOC_H */
