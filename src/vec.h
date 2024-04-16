#ifndef _VEC_H
#define _VEC_H

#include <stddef.h>

/* Opaque vector structure. */
typedef struct Vec Vec;

/* Allocates new vector. Do not forget to free it. */
Vec *vec_alloc(size_t, size_t);

/* Copies items to the end of vector. */
void vec_append(Vec *, const void *, size_t);

/* Returns pointer to vector's items. */
void *vec_items(const Vec *);

/* Finds and deletes item by its index. */
void vec_del(Vec *, size_t);

/* Frees allocated vector. */
void vec_free(Vec *);

/* Sets length to zero. Leaves capacity unchanged. */
void vec_clr(Vec *);

/* Copies items to vector by passed index. */
void vec_ins(Vec *, size_t, const void *, size_t);

/* Returns length of the vector. */
size_t vec_len(const Vec *);

/*
Shrinks vector's capacity. If the flag is set, then determines whether the
shrink is beneficial.
*/
void vec_shrink(Vec *, char);

#endif /* _VEC_H */
