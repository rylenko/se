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

/* TODO: v0.3: make it faster. */
char*
strrstr_slow(const char *haystack, const char *const needle)
{
	const char *match;
	const char *res = NULL;

	/* Check needle is empty */
	if (0 == *needle)
		return (char *)haystack;

	for (;;) {
		/* No more results */
		if (NULL == (match = strstr(haystack, needle)))
			break;
		/* Remember last result */
		res = match;
		haystack = res + 1;
	}
	return (char *)res;
}
