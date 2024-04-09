#ifndef _STR_H
#define _STR_H

#include <stddef.h>

/* Copies passed string to new allocated memory. Do not forget to free it. */
char *str_copy(const char *, size_t);

/* Like `strstr`, but starts from right. */
char *strrstr(const char *, const char *);

#endif /* _STR_H */
