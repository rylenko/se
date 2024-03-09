#ifndef _TOK_H
#define _TOK_H

/* Returns index of next token or given length. */
size_t tok_next(const char *, size_t);

/* Returns index of next from right or given length. */
size_t tok_rnext(const char *, size_t);

#endif /* _TOK_H */
