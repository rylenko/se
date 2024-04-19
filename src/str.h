#ifndef _STR_H
#define _STR_H

#include <stddef.h>

/*
Returns allocated copy of passed string on success and `NULL` on error. Do not
forget to free it.
*/
char *str_copy(const char *, size_t);

/* Like `strstr`, but starts from right. */
char *strrstr(const char *, const char *, size_t);

#endif /* _STR_H */
