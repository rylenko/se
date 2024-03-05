#ifndef _UTIL_H
#define _UTIL_H

/*
Allocates and reads the line including terminal byte to string.

Returns `1` if EOF do not reached, otherwise `0`.
*/
char read_line(FILE *, char **, size_t *);

#endif /* _UTIL_H */
