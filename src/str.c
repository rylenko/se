#include <string.h>
#include "err_alloc.h"
#include "str.h"

char*
str_copy(const char *const str, const size_t len)
{
	/* Allocate and copy memory. Do not forget about null byte */
	char *const copy = err_malloc(len + 1);
	memcpy(copy, str, len);
	copy[len] = 0;
	return copy;
}
