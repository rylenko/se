#ifndef _FILE_H
#define _FILE_H

#include "pos.h"
#include "rows.h"

/*
Internal information about the open file.

If you need a position in the window where each tab byte expands into several
characters, then use a window.
*/
typedef struct {
	Pos pos; /* Current position in the file */
	Rows rows; /* Rows of readed file */
	char is_dirty; /* The file has been modified and not saved */
	char *path; /* Path of readed file. This is where the default save occurs */
} File;

/* Closes file and frees memory. */
void file_close(File *);

/*
Reads the contents of file.

Adds an empty row if there are no rows in the file.

Do not forget to close file.
*/
void file_open(File *, const char *);

#endif /* _FILE_H */
