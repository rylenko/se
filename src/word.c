#include <ctype.h>
#include <stddef.h>
#include "word.h"

size_t
word_next(const char *const s, const size_t len)
{
	size_t i;
	char space_visited = 0;
	for (i = 0; i + 1 < len; i++) {
		if (!isspace(s[i]) && space_visited) {
			return i;
		} else if (isspace(s[i]) && !space_visited) {
			space_visited = 1;
		}
	}
	return len;
}

size_t
word_rnext(const char *const s, const size_t len)
{
	size_t i;
	char space_visited = 0;
	char prev_word_visited = 0;
	if (len > 1) {
		for (i = len - 1;; i--) {
			if (!isspace(s[i]) && space_visited) {
				prev_word_visited = 1;
			} else if (isspace(s[i]) && !space_visited) {
				space_visited = 1;
			} else if (isspace(s[i]) && space_visited && prev_word_visited) {
				return i + 1;
			}
			if (i == 0) {
				break;
			}
		}
	}
	return 0;
}
