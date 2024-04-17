#include <string.h>
#include "alloc.h"
#include "str.h"

char*
str_copy(const char *const str, const size_t len)
{
	/* Allocate and copy memory. Do not forget about null byte */
	char *const copy = malloc_err(len + 1);
	memcpy(copy, str, len);
	copy[len] = 0;
	return copy;
}

char*
strrstr(const char *haystack, const char *const needle, const size_t len)
{
	const char *ptr = haystack + len;
	const size_t needle_len = strlen(needle);

	/* Check needle is empty */
	if (0 == *needle)
		return (char *)haystack;

	do {
		ptr--;
		/* Compare current shifted part with needle */
		if (strncmp(ptr, needle, needle_len) == 0)
			return (char *)ptr;
	} while (ptr > haystack);
	return NULL;
}
