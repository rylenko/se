#ifndef _FILE_H
#define _FILE_H

#include <stddef.h>

/* Alias for opaque struct of opened file. */
typedef struct File File;

/* Finds line by passed index and absorbs next line. */
void file_absorb_next_line(File *, size_t);

/* Finds line by its index and breaks it at passed position. */
void file_break_line(File *, size_t, size_t);

/* Closes file and frees memory. */
void file_close(File *);

/* Deletes character in file's line at passed position. */
void file_del_char(File *, size_t, size_t);

/* Inserts character to the file's line at passed position. */
void file_ins_char(File *, size_t, size_t, char);

/* Inserts empty line at index. */
void file_ins_empty_line(File *, size_t);

/* Checks that file is dirty. */
char file_is_dirty(const File *);

/* Deletes line by its index. */
void file_del_line(File *, size_t idx);

/* Finds line by passed index and returns its raw content. */
const char *file_line_cont(const File *, size_t);

/* Finds line by passed index and returns its length. */
size_t file_line_len(const File *, size_t);

/* Finds line by passed index and returns its render. */
const char *file_line_render(const File *, size_t);

/* Finds line by passed index and returns its render len. */
size_t file_line_render_len(const File *, size_t);

/* Returns lines count of opened file. */
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

/*
Searches forward from passed line index and passes line position. Modifies
index and position if results found. Do nothing otherwise.

Returns 1 if result found, otherwise 0.
*/
char file_search_fwd(const File *, size_t *, size_t *, const char *);

#endif /* _FILE_H */
