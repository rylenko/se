/* err */
#include <err.h>
/* EXIT_FAILURE, NULL, malloc */
#include <stdlib.h>
/* strcpy, strlen */
#include <string.h>
#include "str.h"

char*
str_clone(const char *const str)
{
	const size_t len = strlen(str);
	char *const ret = malloc(len + 1);
	if (NULL == ret) {
		err(EXIT_FAILURE, "Failed to clone string with length %zu", len);
	}
	strcpy(ret, str);
	return ret;
}
