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
	const char *ptr;
	size_t query_len;

	/* Check query is empty */
	if (0 == *query)
		return (char *)str;
	query_len = strlen(query);

	for (ptr = str + len - query_len; ptr >= str; ptr--) {
		/* Compare current shifted part with needle */
		ret = strncmp(ptr, query, query_len);
		if (0 == ret)
			return (char *)ptr;
	}
	return NULL;
}

char*
str_search(const char *str, const char *const query, const size_t len)
{
	int ret;
	const char *ptr;
	size_t query_len;

	/* Check needle is empty */
	if (0 == *query)
		return (char *)str;
	query_len = strlen(query);

	/* Check searching has the meaning */
	if (len < query_len)
		return NULL;

	for (ptr = str; ptr < str + len; ptr++) {
		/* Compare current shifted part with needle */
		ret = strncmp(ptr, query, query_len);
		if (0 == ret)
			return (char *)ptr;
	}
	return NULL;
}
