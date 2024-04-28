#include <ctype.h>
#include <stddef.h>
#include "word.h"

size_t
word_next(const char *const str, const size_t len)
{
	size_t i;
	char space_visited = 0;

	for (i = 0; i < len; i++) {
		/* First appearance of non-space after spaces */
		if (!isspace(str[i]) && space_visited)
			return i;
		/* First appearance of space */
		else if (isspace(str[i]) && !space_visited)
			space_visited = 1;
	}
	return len;
}

size_t
word_rnext(const char *const str, size_t pos)
{
	char space_visited = 0;
	char prev_word_visited = 0;

	while (pos-- > 0) {
		/* First appearance of non space after spaces */
		if (!isspace(str[pos]) && space_visited)
			prev_word_visited = 1;
		/* First appearance of space */
		else if (isspace(str[pos]) && !space_visited)
			space_visited = 1;
		/* Second appearance of spaces after word */
		else if (isspace(str[pos]) && prev_word_visited)
			/* Start of word before second spaces */
			return pos + 1;
	}
	return 0;
}
