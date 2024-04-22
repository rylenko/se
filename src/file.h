#ifndef _FILE_H
#define _FILE_H

#include <stddef.h>

/* File traversal direction. Used, for example, in search. */
enum Dir {
	DIR_BWD,
	DIR_FWD,
};

/* Alias for opaque struct of opened file. */
typedef struct File File;

/*
Finds line by passed index and absorbs next line.

Returns 0 on success and -1 on error.
*/
int file_absorb_next_line(File *, size_t);

/*
Finds line by its index and breaks it at passed position.

Returns 0 on success and -1 on error.
*/
int file_break_line(File *, size_t, size_t);

/* Closes file and frees memory. */
void file_close(File *);

/*
Deletes character in file's line at passed position.

Returns 0 on success and -1 on error.
*/
int file_del_char(File *, size_t, size_t);

/*
Inserts character to the file's line at passed position.

Returns 0 on success and -1 on error.
*/
int file_ins_char(File *, size_t, size_t, char);

/*
Inserts empty line at index.

Returns 0 on success and -1 on error.
*/
int file_ins_empty_line(File *, size_t);

/* Checks that file is dirty. */
char file_is_dirty(const File *);

/*
Deletes line by its index.

Returns 0 on success and -1 on error.

Sets `ENOSYS` if there is one last line left that cannot be deleted.
*/
int file_del_line(File *, size_t idx);

/*
Finds line by passed index and returns its raw chars.

Returns pointer to characters on success and `NULL` on error.
*/
const char *file_line_chars(const File *, size_t);

/*
Finds line by passed index and writes its length to passed pointer.

Returns 0 on success and -1 on error.
*/
int file_line_len(const File *, size_t, size_t *);

/*
Finds line by passed index and returns its render.

Returns pointer to characters on success and `NULL` on error.
*/
const char *file_line_render(const File *, size_t);

/*
Finds line by passed index and returns its render len.

Returns 0 on success and -1 on error.
*/
int file_line_render_len(const File *, size_t, size_t *);

/* Returns lines count of opened file. */
size_t file_lines_cnt(const File *);

/*
Reads the contents of file. Adds an empty line if there are no lines in the
file. Do not forget to close file.

Returns pointer to opaque struct on success or `NULL` on error.
*/
File *file_open(const char *);

/* Gets path of opened file. */
const char *file_path(const File *);

/*
Saves file to passed path. Saves to opened file's path if argument is `NULL`.

Returns written bytes count and 0 on error.
*/
size_t file_save(File *, const char *);

/*
Saves file to spare directory with generated path. Useful if no privileges.

Writes final path to passed buffer up to passed length. The buffer must have a
capacity one greater than the length for a null byte.

Returns written bytes count on success and 0 on error.

Sets `ENOBUFS` if path buffer too small.
*/
size_t file_save_to_spare_dir(File *, char *, size_t);

/*
Searches in passed direction from passed line index and passes line position.
Modifies index and position.

Temporarily changes file if direction is backward, but then restores it.

Returns 1 if result found, 0 if no result and -1 on error.
*/
int file_search(File *, size_t *, size_t *, const char *, enum Dir);

#endif /* _FILE_H */
