#include <ctype.h>
#include <stdlib.h>
#include "tok.h"

char*
tok_next(const char *str)
{
	char first_is_alnum;

	if (*str) {
		first_is_alnum = isalnum(*str);
		for (str++; *str; str++) {
			if (first_is_alnum ^ isalnum(*str)) {
				return (char*)str;
			}
		}
	}
	return NULL;
}
