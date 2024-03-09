#ifndef _TOK_H
#define _TOK_H

/*
If first character `isalnum`, then it will find other symbols. Otherwise will
find `isalnum`.
*/
char *tok_next(const char *);

#endif /* _TOK_H */
