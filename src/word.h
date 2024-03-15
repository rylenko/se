#ifndef _WORD_H
#define _WORD_H

/* Returns index of next token, otherwise given length. */
size_t word_next(const char *, const size_t);

/* Returns index of next word from right, otherwise 0. */
size_t word_rnext(const char *, const size_t);

#endif /* _WORD_H */
