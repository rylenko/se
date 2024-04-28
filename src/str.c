#include <string.h>
#include <stdlib.h>
#include "str.h"

char*
str_copy(const char *const str, const size_t len)
{
	/* Allocate memory. Do not forget about null byte */
	char *const copy = malloc(len + 1);
	if (NULL == copy)
		return NULL;

	/* Copy memory. Do not forget about null byte */
	copy[len] = 0;
	return memcpy(copy, str, len);
}

char*
strnstr(const char *haystack, const char *const needle, const size_t len)
{
	int ret;
	const char *const end = haystack + len;
	const size_t needle_len = strlen(needle);

	/* Check needle is empty */
	if (0 == *needle)
		return (char *)haystack;

	for (; haystack < end; haystack++) {
		/* Compare current shifted part with needle */
		ret = strncmp(haystack, needle, needle_len);
		if (0 == ret)
			return (char *)haystack;
	}
	return NULL;
}

char*
strrstr(const char *const haystack, const char *const needle, const size_t len)
{
	int ret;
	const char *ptr = haystack + len;
	const size_t needle_len = strlen(needle);

	/* Check needle is empty */
	if (0 == *needle)
		return (char *)haystack;

	do {
		ptr--;

		/* Compare current shifted part with needle */
		ret = strncmp(ptr, needle, needle_len);
		if (0 == ret)
			return (char *)ptr;
	} while (ptr > haystack);
	return NULL;
}
