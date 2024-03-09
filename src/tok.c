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
tok_next(const char *s, size_t len)
{
	size_t i;
	char first_is_sep;

	if (len > 0) {
		first_is_sep = is_sep(s[0]);
		for (i = 1; i < len; i++) {
			if (first_is_sep ^ is_sep(s[i])) {
				return i;
			}
		}
	}
	return len;
}

size_t
tok_rnext(const char *s, size_t len)
{
	char first_is_sep;

	if (len > 0) {
		first_is_sep = is_sep(s[len - 1]);
		for (; len > 0; len--) {
			if (first_is_sep ^ is_sep(s[len - 1])) {
				return len - 1;
			}
		}
	}
	return len;
}