#include <err.h>
#include <stdlib.h>
#include "alloc.h"

/* Validates allocated pointer against `NULL`. Returns passed pointer. */
static void *alloc_validate(void *);

static void*
alloc_validate(void *const ptr)
{
	if (NULL == ptr)
		err(EXIT_FAILURE, "Failed to allocate memory");
	return ptr;
}

void*
calloc_err(const size_t cnt, const size_t size)
{
	return alloc_validate(calloc(cnt, size));
}

void*
malloc_err(const size_t size)
{
	return alloc_validate(malloc(size));
}

void*
realloc_err(void *const ptr, const size_t size)
{
	return alloc_validate(realloc(ptr, size));
}
