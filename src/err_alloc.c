#include <err.h>
#include <stdlib.h>
#include "err_alloc.h"

/* Validates allocated pointer against `NULL`. Returns passed pointer. */
static void *err_alloc_validate(void *);

static void*
err_alloc_validate(void *const ptr)
{
	if (NULL == ptr)
		err(EXIT_FAILURE, "Failed to allocate memory");
	return ptr;
}

void*
err_calloc(const size_t count, const size_t size)
{
	return err_alloc_validate(calloc(count, size));
}

void*
err_malloc(const size_t size)
{
	return err_alloc_validate(malloc(size));
}

void*
err_realloc(void *const ptr, const size_t size)
{
	return err_alloc_validate(realloc(ptr, size));
}
