#include <ctype.h>
#include <stdlib.h>
#include "tok.h"

/* Check character is separator. */
static char is_sep(char ch);

static char
is_sep(char ch)
{
	return ispunct(ch) || isspace(ch);
}

char*
tok_next(const char *str)
{
	char first_is_sep;

	if (*str) {
		first_is_sep = is_sep(*str);
		for (str++; *str; str++) {
			if (first_is_sep ^ is_sep(*str)) {
				return (char*)str;
			}
		}
	}
	return NULL;
}
