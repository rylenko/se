#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "str_util.h"

char*
str_clone(const char *str)
{
	size_t len = strlen(str);
	char *ret = malloc(len + 1);
	if (!ret) {
		err("Failed to clone string with length %zu:", len);
	}
	strcpy(ret, str);
	return ret;
}
