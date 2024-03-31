#ifndef _PATH_H
#define _PATH_H

/* Like `basename` from `libgen.h`, but accepts `const char *`. */
const char *path_get_filename(const char *);

#endif /* _PATH_H */
