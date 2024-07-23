#include <string.h>
#include <stdlib.h>
#include "cfg.h"
#include "str.h"

char*
str_copy(const char *const str, const size_t len)
{
	char *copy;

	/* Allocate memory. Do not forget about null byte. */
	copy = malloc(len + 1);
	if (NULL == copy)
		return NULL;

	/* Copy memory. Do not forget about null byte. */
	copy[len] = '\0';
	return memcpy(copy, str, len);
}

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
