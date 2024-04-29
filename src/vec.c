#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "vec.h"

struct vec {
	char *items; /* Pointer to the beginning of dynamic array with items  */
	size_t item_size; /* Size of item in dynamic array */
	size_t len; /* Length of dynamic array */
	size_t cap; /* Capacity of dynamic array */
	size_t cap_step; /* Step of growing and shrinking of dynamic array */
};

/*
Grows vector's capacity for new length if needed.

Returns 0 on success and -1 on error.
*/
static int vec_grow_if_needed(struct vec *, size_t);

/*
Reallocates vector with new capacity.

Returns 0 on success and -1 on error.
*/
static int vec_realloc(struct vec *, size_t);

/*
Like default inserting function, but with formatting using variadic list.

Returns 0 on success and -1 on error.

Sets `ENOSUP` if vector does not stores characters.
*/
static int vec_ins_fmt_va(struct vec *, size_t, const char *, va_list);

struct vec*
vec_alloc(const size_t item_size, const size_t cap_step)
{
	struct vec *vec;

	/* Allocate opaque struct */
	vec = calloc(1, sizeof(*vec));
	if (NULL == vec)
		return NULL;

	/* Initialize some fields */
	vec->item_size = item_size;
	vec->cap_step = cap_step;
	return vec;
}

int
vec_append(struct vec *const vec, const void *const items, const size_t len)
{
	int ret;

	/* Insert to the end of vector */
	ret = vec_ins(vec, vec->len, items, len);
	return ret;
}

int
vec_append_fmt(struct vec *const vec, const char *const fmt, ...)
{
	int ret;
	va_list args;

	/* Collect arguments and insert to the end of vector */
	va_start(args, fmt);
	ret = vec_ins_fmt_va(vec, vec->len, fmt, args);
	va_end(args);
	return ret;
}

size_t
vec_cap(const struct vec *const vec)
{
	return vec->cap;
}

void*
vec_get(const struct vec *const vec, const size_t idx)
{
	/* Validate index */
	if (idx >= vec->len) {
		errno = EINVAL;
		return NULL;
	}
	return &vec->items[idx * vec->item_size];
}

static int
vec_grow_if_needed(struct vec *const vec, const size_t new_len)
{
	int ret;
	size_t new_cap;

	/* No need to grow */
	if (new_len <= vec->cap)
		return 0;

	/* Get optimal new capacity */
	new_cap = MAX(vec->cap + vec->cap_step, new_len);

	/* Grow with new capacity */
	ret = vec_realloc(vec, new_cap);
	return ret;
}

int
vec_ins(
	struct vec *const vec,
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

int
vec_ins_fmt(
	struct vec *const vec,
	const size_t idx,
	const char *const fmt,
	...
) {
	int ret;
	va_list args;

	/* Collect arguments and use inner function */
	va_start(args, fmt);
	ret = vec_ins_fmt_va(vec, idx, fmt, args);
	va_end(args);
	return ret;
}

static int
vec_ins_fmt_va(
	struct vec *const vec,
	const size_t idx,
	const char *const fmt,
	va_list args
) {
	int len;
	int ret;
	char buf[256];

	/* Check that vector stores characters */
	if (sizeof(char) != vec->item_size) {
		errno = ENOTSUP;
		return -1;
	}

	/* Format arguments */
	len = vsnprintf(buf, sizeof(buf), fmt, args);
	if (len < 0 || (size_t)len >= sizeof(buf))
		return -1;

	/* Insert formatted data to the vector */
	ret = vec_ins(vec, idx, buf, len);
	if (-1 == ret)
		return -1;
	return len;
}

void
vec_free(struct vec *const vec)
{
	free(vec->items);
	free(vec);
}

void*
vec_items(const struct vec *const vec)
{
	return vec->items;
}

size_t
vec_len(const struct vec *const vec)
{
	return vec->len;
}

static int
vec_realloc(struct vec *const vec, const size_t new_cap)
{
	/* Reallocate items and update the capacity */
	vec->cap = new_cap;
	vec->items = realloc(vec->items, new_cap * vec->item_size);
	return NULL == vec->items ? -1 : 0;
}

int
vec_remove(struct vec *const vec, const size_t idx, void *const item)
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
	return ret;
}

int
vec_set_len(struct vec *const vec, const size_t len)
{
	/* Validate new length */
	if (len > vec->cap) {
		errno = EINVAL;
		return -1;
	}
	vec->len = len;
	return 0;
}

int
vec_shrink(struct vec *const vec, const char to_fit)
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
