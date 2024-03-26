#include <err.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

char*
str_copy(const char *const str, const size_t len)
{
	/* Allocate memory for copy. Do not forget about null byte */
	char *const copy = malloc(len + 1);
	if (NULL == copy)
		err(EXIT_FAILURE, "Failed to allocate copied string with length %zu", len);

	/* Copy string and set null byte */
	memcpy(copy, str, len);
	copy[len] = 0;
	return copy;
}
