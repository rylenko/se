#ifndef _VEC_H
#define _VEC_H

#include <stddef.h>

/* Opaque vector structure. */
typedef struct Vec Vec;

/*
Allocates new vector. Do not forget to free it.

Returns pointer to opaque vector on success and `NULL` on error.
*/
Vec *vec_alloc(size_t, size_t);

/*
Copies items to the end of vector.

Returns 0 on success and -1 on error.
*/
int vec_append(Vec *, const void *, size_t);

/* Returns capacity of the vector. */
size_t vec_cap(const Vec *);

/*
Gets vector's item by index.

Returns pointer to item on success and `NULL` on error.
*/
void *vec_get(const Vec *, size_t);

/* Returns pointer to vector's items. */
void *vec_items(const Vec *);

/* Frees allocated vector. */
void vec_free(Vec *);

/*
Copies items to vector by passed index.

Returns 0 on success and -1 on error.
*/
int vec_ins(Vec *, size_t, const void *, size_t);

/* Returns length of the vector. */
size_t vec_len(const Vec *);

/*
Finds and removes item by its index.

If `errno` is not equal to `EINVAL`, then the item was removed and written to
the passed pointer if it's not `NULL`.

Returns 0 on success and -1 on error.
*/
int vec_remove(Vec *, size_t, void *);

/*
Sets new length. Must not be greater than capacity. Capacity remains the same.

Returns 0 on success and -1 on error.
*/
int vec_set_len(Vec *, size_t);

/*
Shrinks vector's capacity. Determines whether the shrink is beneficial if flag
was not set.

Returns 0 on success and -1 on error.
*/
int vec_shrink(Vec *, char);

#endif /* _VEC_H */
