#ifndef _FILE_H
#define _FILE_H

#include <stddef.h>
#include "line.h"

/* Alias for opaque struct of opened file. */
typedef struct File File;

/* Closes file and frees memory. */
void file_close(File *);

/* Deletes line from the file. */
void file_del(File *, size_t);

/* Gets line by its index. */
Line *file_get(const File *, size_t);

/* Checks that file is dirty. */
char file_is_dirty(const File *);

/* Gets lines count in the file. */
size_t file_lines_cnt(const File *);

/*
Reads the contents of file.

Adds an empty line if there are no lines in the file.

Do not forget to close file.
*/
File *file_open(const char *);

/* Gets path of opened file. */
const char *file_path(const File *);

/*
Saves file to passed path. Saves to opened file's path if argument is `NULL`.

Returns written bytes count or zero if failed to save the file.
*/
size_t file_save(File *, const char *);

/*
Saves file to spare directory with generated path. Useful if no privileges.

Writes final path to passed buffer up to passed length. The buffer must have a
capacity one greater than the length for a null byte.

Returns written bytes count.
*/
size_t file_save_to_spare_dir(File *, char *, size_t);

#endif /* _FILE_H */
