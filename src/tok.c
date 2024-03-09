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

size_t
tok_next(const char *s)
{
	size_t i;
	char first_is_sep;

	if (s[0]) {
		first_is_sep = is_sep(s[0]);
		for (i = 1; s[i]; i++) {
			if (first_is_sep ^ is_sep(s[i])) {
				return i;
			}
		}
	}
	return 0;
}
