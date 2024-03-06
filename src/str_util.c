/* malloc */
#include <stdlib.h>
/* strcpy, strlen */
#include <string.h>
/* err */
#include "err.h"
/* str_clone */
#include "str_util.h"

char*
str_clone(const char *str)
{
	size_t len = strlen(str);
	char *ret = malloc(len + 1);
	if (!ret)
		err("Failed to clone string with length %zu:", len);
	strcpy(ret, str);
	return ret;
}
