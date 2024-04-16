#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "math.h"
#include "vec.h"

struct Vec {
	char *items; /* Pointer to the beginning of dynamic array with items  */
	size_t item_size; /* Size of item in dynamic array */
	size_t len; /* Length of dynamic array */
	size_t cap; /* Capacity of dynamic array */
	size_t cap_step; /* Step of growing and shrinking of dynamic array */
};

/* Grows vector's capacity for new length if needed. */
static void vec_grow_if_needed(struct Vec *, size_t);

/* Reallocates vector with new capacity. */
static void vec_realloc(struct Vec *, size_t);

struct Vec*
vec_alloc(const size_t item_size, const size_t cap_step)
{
	struct Vec *const vec = calloc_err(1, sizeof(*vec));
	vec->item_size = item_size;
	vec->cap_step = cap_step;
	return vec;
}

void
vec_append(struct Vec *const vec, const void *const items, const size_t len)
{
	vec_ins(vec, vec->len, items, len);
}

void
vec_clr(struct Vec *const vec)
{
	vec->len = 0;
}

void
vec_del(struct Vec *const vec, const size_t idx)
{
	/* Validate index */
	assert(idx < vec->len);

	/* Move items left to overlap bytes */
	memmove(
		&vec->items[idx * vec->item_size],
		&vec->items[(idx + 1) * vec->item_size],
		(--vec->len - idx) * vec->item_size
	);
	/* Shrink vector if there is too much free space */
	vec_shrink(vec, 1);
}

static void
vec_grow_if_needed(struct Vec *const vec, const size_t new_len)
{
	if (new_len > vec->cap)
		vec_realloc(vec, MAX(vec->cap + vec->cap_step, new_len));
}

void
vec_ins(
	struct Vec *const vec,
	const size_t idx,
	const void *const items,
	const size_t len
) {
	/* Validate index */
	assert(idx <= vec->len);

	/* Grow if there is no space for new items */
	vec_grow_if_needed(vec, vec->len + len);
	/* Reserve space for new items */
	memmove(
		&vec->items[(idx + len) * vec->item_size],
		&vec->items[idx * vec->item_size],
		(vec->len - idx) * vec->item_size
	);
	vec->len += len;
	/* Copy new item */
	memcpy(&vec->items[idx * vec->item_size], items, len * vec->item_size);
}

void*
vec_items(const struct Vec *const vec)
{
	return vec->items;
}

void
vec_free(struct Vec *const vec)
{
	free(vec->items);
	free(vec);
}

size_t
vec_len(const struct Vec *const vec)
{
	return vec->len;
}

static void
vec_realloc(struct Vec *const vec, const size_t new_cap)
{
	/* Reallocates items and updates the capacity */
	vec->items = realloc_err(vec->items, new_cap * vec->item_size);
	vec->cap = new_cap;
}

void
vec_shrink(struct Vec *const vec, const char if_needed)
{
	if (0 == vec->len && vec->cap > 0) {
		/* Free allocated items if vector is empty */
		free(vec->items);
		vec->items = NULL;
		vec->cap = 0;
	} else if (
		/* Reallocate items to equate capacity to length */
		(if_needed && vec->len < vec->cap)
		/* Reallocate if there is too much unused capacity */
		|| vec->len + vec->cap_step <= vec->cap
	) {
		vec_realloc(vec, vec->len);
	}
}
