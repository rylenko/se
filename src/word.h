#ifndef _WORD_H
#define _WORD_H

#include <stddef.h>

/*
 * Returns index of next word, otherwise given length.
 */
size_t word_next(const char *, size_t);

/*
 * Returns index of next word from right, otherwise zero.
 */
size_t word_rnext(const char *, size_t);

#endif /* _WORD_H */
