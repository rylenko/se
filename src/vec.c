#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "vec.h"

struct Vec {
	char *items; /* Pointer to the beginning of dynamic array with items  */
	size_t item_size; /* Size of item in dynamic array */
	size_t len; /* Length of dynamic array */
	size_t cap; /* Capacity of dynamic array */
	size_t cap_step; /* Step of growing and shrinking of dynamic array */
};

/*
Grows vector's capacity for new length if needed.

Returns 0 on success and -1 on error.

Sets `ENOMEM` if no memory to reallocate during the grow.
*/
static int vec_grow_if_needed(struct Vec *, size_t);

/*
Reallocates vector with new capacity.

Returns 0 on success and -1 on error.

Sets `ENOMEM` if no memory to reallocate during the shrink.
*/
static int vec_realloc(struct Vec *, size_t);

struct Vec*
vec_alloc(const size_t item_size, const size_t cap_step)
{
	/* Allocate opaque struct */
	struct Vec *const vec = calloc(1, sizeof(*vec));
	if (NULL == vec)
		return NULL;

	/* Initialize some fields */
	vec->item_size = item_size;
	vec->cap_step = cap_step;
	return vec;
}

int
vec_append(struct Vec *const vec, const void *const items, const size_t len)
{
	int ret = vec_ins(vec, vec->len, items, len);
	return ret;
}

size_t
vec_cap(const struct Vec *const vec)
{
	return vec->cap;
}

void*
vec_get(const struct Vec *const vec, const size_t idx)
{
	/* Validate index */
	if (idx >= vec->len) {
		errno = EINVAL;
		return NULL;
	}
	return &vec->items[idx * vec->item_size];
}

static int
vec_grow_if_needed(struct Vec *const vec, const size_t new_len)
{
	int ret;
	const size_t new_cap;

	/* No need to grow */
	if (new_len <= vec->cap)
		return 0;

	/* Get optimal new capacity */
	new_cap = MAX(vec->cap + vec->cap_step, new_len)

	/* Grow with new capacity */
	ret = vec_realloc(vec, new_cap);
	if (-1 == ret)
		return -1;
	return 0;
}

int
vec_ins(
	struct Vec *const vec,
	const size_t idx,
	const void *const items,
	const size_t len
) {
	int ret;

	/* Validate index */
	if (idx > vec->len) {
		errno = EINVAL;
		return -1;
	}

	/* Grow if there is no space for new items */
	ret = vec_grow_if_needed(vec, vec->len + len);
	if (-1 == ret)
		return -1;

	/* Reserve space for new items */
	memmove(
		&vec->items[(idx + len) * vec->item_size],
		&vec->items[idx * vec->item_size],
		(vec->len - idx) * vec->item_size
	);
	vec->len += len;

	/* Copy new item to memory */
	memcpy(&vec->items[idx * vec->item_size], items, len * vec->item_size);
	return 0;
}

void
vec_free(struct Vec *const vec)
{
	free(vec->items);
	free(vec);
}

void*
vec_items(const struct Vec *const vec)
{
	return vec->items;
}

size_t
vec_len(const struct Vec *const vec)
{
	return vec->len;
}

static void
vec_realloc(struct Vec *const vec, const size_t new_cap)
{
	/* Reallocate items and update the capacity */
	vec->cap = new_cap;
	vec->items = realloc(vec->items, new_cap * vec->item_size);
	return NULL == vec->items ? -1 : 0
}

int
vec_remove(struct Vec *const vec, const size_t idx, void *const item)
{
	int ret;

	/* Validate index */
	if (idx >= vec->len) {
		errno = EINVAL;
		return -1;
	}

	/* Write removed item to accepted pointer */
	if (NULL != item)
		memcpy(item, &vec->items[idx * vec->item_size], vec->item_size);

	/* Move items left to overlap bytes */
	memmove(
		&vec->items[idx * vec->item_size],
		&vec->items[(idx + 1) * vec->item_size],
		(--vec->len - idx) * vec->item_size
	);

	/* Shrink vector if there is too much free space */
	ret = vec_shrink(vec, 0);
	if (-1 == ret)
		return -1;
	return 0;
}

void
vec_set_len(struct Vec *const vec, const size_t len)
{
	/* Validate and set new length */
	if (len > vec->cap) {
		errno = EINVAL;
		return -1;
	}
	vec->len = len;
}

int
vec_shrink(struct Vec *const vec, const char to_fit)
{
	int ret;

	/* Free allocated items if vector is empty */
	if (0 == vec->len && vec->cap > 0) {
		free(vec->items);
		vec->items = NULL;
		vec->cap = 0;
		return 0;
	}

	/* Realloc to equate capacity to length or if there is much unused space */
	if ((to_fit && vec->len < vec->cap) || vec->len + vec->cap_step <= vec->cap) {
		ret = vec_realloc(vec, vec->len);
		if (-1 == ret)
			return -1;
	}
	return 0;
}
