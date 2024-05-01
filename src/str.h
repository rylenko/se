#ifndef _STR_H
#define _STR_H

#include <stddef.h>

/*
Returns allocated copy of passed string on success and `NULL` on error. Do not
forget to free it.

Sets `ENOMEM` if no memory to allocate a copy of the string.
*/
char *str_copy(const char *, size_t);

/*
Returns the number of characters to which the character should be expanded.

Passed number specifies current position in the text.
*/
size_t str_exp(const char, size_t);

#endif /* _STR_H */
