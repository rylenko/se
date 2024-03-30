#ifndef _FILE_H
#define _FILE_H

#include "lines.h"

/* Internal information about the open file. */
typedef struct {
	Lines lines; /* Lines of readed file. There is always at least one line */
	char is_dirty; /* The file has been modified and not saved */
	char *path; /* Path of readed file. This is where the default save occurs */
} File;

/* Closes file and frees memory. */
void file_close(File *);

/*
Reads the contents of file.

Adds an empty line if there are no lines in the file.

Do not forget to close file.
*/
void file_open(File *, const char *);

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
