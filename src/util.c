#include "err.h"
#include "util.h"

#define LINE_REALLOC_STEP (256)

char
read_line(FILE *f, char **buf, size_t* len)
{
	size_t cap = LINE_REALLOC_STEP;
	int ch;
	*len = 0;

	/* Allocate initial buffer */
	if (!(*buf = malloc(cap)))
		err("Failed to allocate initial buffer for readed line:");

	while (true) {
		/* Reallocate buffer with new capacity */
		if (len == cap) {
			cap += LINE_REALLOC_STEP;
			if (!(*buf = realloc(*buf, cap)))
				err("Failed to allocate %zu bytes for readed line:", cap);
		}

		/* Read new character */
		ch = fgetc(f);
		if (ch == '\n' || ch == EOF) {
			(*buf)[*len] = 0;
			return ch == '\n';
		}
		(*buf)[*len++] = ch;
	}
}
