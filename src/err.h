#ifndef _ERR_H
#define _ERR_H

/*
Prints message to stderr.

Also prints an errno message if there is a `:` at the end.
*/
void err(const char *, ...);

#endif /* _ERR_H */
