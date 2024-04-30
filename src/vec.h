#ifndef _VEC_H
#define _VEC_H

#include <stddef.h>

/* Opaque vector structure. */
typedef struct vec Vec;

/*
Allocates new vector. Do not forget to free it.

Returns pointer to opaque vector on success and `NULL` on error.
*/
Vec *vec_alloc(size_t, size_t);

/*
Copies items to the end of vector. Grows capacity if there is not enough space
to append.

Returns 0 on success and -1 on error.
*/
int vec_append(Vec *, const void *, size_t);

/*
Like default appending function, but with string formatting. Grows capacity if
there is not enough space to append.

Returns formatted length on success and -1 on error.

Sets `ENOSUP` if vector does not stores characters.
*/
int vec_append_fmt(Vec *, const char *, ...);

/* Returns capacity of the vector. */
size_t vec_cap(const Vec *);

/*
Gets vector's item by index.

Returns pointer to item on success and `NULL` on error.

Sets `EINVAL` if index is invalid.
*/
void *vec_get(const Vec *, size_t);

/* Returns pointer to vector's items. */
void *vec_items(const Vec *);

/* Frees allocated vector. */
void vec_free(Vec *);

/*
Copies items to vector by passed index. Grows capacity if there is not enough
space to insert.

Returns 0 on success and -1 on error.

Sets `EINVAL` if index is invalid.
*/
int vec_ins(Vec *, size_t, const void *, size_t);

/*
Like default inserting function, but with string formatting. Grows capacity if
there is not enough space to insert.

Returns formatted length on success and -1 on error.

Sets `ENOSUP` if vector does not stores characters.
*/
int vec_ins_fmt(Vec *, size_t, const char *, ...);

/* Returns length of the vector. */
size_t vec_len(const Vec *);

/*
Finds and removes item by its index. Shrinks capacity if too much space is
unused.

Returns 0 on success and -1 on error.

Sets `EINVAL` if index is invalid.

If `errno` is not equal to `EINVAL`, then the item was removed and written to
the passed pointer if it's not `NULL`.
*/
int vec_remove(Vec *, size_t, void *);

/*
Sets length and leaves the capacity unchanged, so shrink the capacity if
needed.

Returns 0 on success and -1 on error.

Sets `EINVAL` if length is greater than capacity.
*/
int vec_set_len(Vec *, size_t);

/*
Shrinks capacity if too much space is unused.

Returns 0 on success and -1 on error.
*/
int vec_shrink_if_needed(Vec *);

#endif /* _VEC_H */
