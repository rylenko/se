#include <string.h>
#include <stdlib.h>
#include "cfg.h"
#include "str.h"

size_t
str_exp(const char ch, const size_t pos)
{
	switch (ch) {
	case '\t':
		return CFG_TAB_SIZE - pos % CFG_TAB_SIZE;
	default:
		return 1;
	}
}

char*
str_copy(const char *const str, const size_t len)
{
	char *copy;

	/* Allocate memory. Do not forget about null byte */
	copy = malloc(len + 1);
	if (NULL == copy)
		return NULL;

	/* Copy memory. Do not forget about null byte */
	copy[len] = 0;
	return memcpy(copy, str, len);
}

char*
str_rsearch(const char *const str, const char *const query, const size_t len)
{
	int ret;
	const char *ptr = str + len;
	size_t query_len;

	/* Check needle is empty */
	if (0 == *query)
		return (char *)str;
	query_len = strlen(query);

	do {
		ptr--;

		/* Compare current shifted part with needle */
		ret = strncmp(ptr, query, query_len);
		if (0 == ret)
			return (char *)ptr;
	} while (ptr > str);
	return NULL;
}

char*
str_search(const char *str, const char *const query, const size_t len)
{
	int ret;
	const char *const end = str + len;
	size_t query_len;

	/* Check needle is empty */
	if (0 == *query)
		return (char *)str;
	query_len = strlen(query);

	for (; str < end; str++) {
		/* Compare current shifted part with needle */
		ret = strncmp(str, query, query_len);
		if (0 == ret)
			return (char *)str;
	}
	return NULL;
}
