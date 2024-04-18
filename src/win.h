#ifndef _WIN_H
#define _WIN_H

#include <stddef.h>
#include <sys/ioctl.h>
#include "file.h"
#include "vec.h"

/* Alias for opaque struct with window parameters. */
typedef struct Win Win;

/* Closes the window. */
void win_close(Win *);

/* Gets current line's index. */
size_t win_curr_line_idx(const Win *);

/* Gets current line char's index. */
size_t win_curr_line_char_idx(const Win *);

/* Breaks current line at cursor position. */
void win_break_line(Win *);

/* Deletes character before the cursor. */
void win_del_char(Win *);

/*
Deletes the passed number of lines starting from the current one.

Returns 0 on success and -1 if there is only one line which cannot be deleted.
*/
int win_del_line(Win *, size_t);

/* Draws cursor. */
void win_draw_cur(const Win *, Vec *);

/* Draws window rows. */
void win_draw_lines(const Win *, Vec *);

/* Checks that opened file is dirty. */
char win_file_is_dirty(const Win *);

/* Returns opened file's path. */
const char *win_file_path(const Win *);

/* Handles signal. */
void win_handle_signal(Win *, int);

/* Inserts character to the file. */
void win_ins_char(Win *, char);

/* Inserts empty line below several times. */
void win_ins_empty_line_below(Win *, size_t);

/* Inserts empty line on top of cursor several times. */
void win_ins_empty_line_on_top(Win *, size_t);

/* Move down several times. */
void win_mv_down(Win *, size_t);

/* Move left several times. */
void win_mv_left(Win *, size_t);

/* Move right several times. */
void win_mv_right(Win *, size_t);

/* Moves to begin of first line. */
void win_mv_to_begin_of_file(Win *);

/* Moves to begin of current line. */
void win_mv_to_begin_of_line(Win *);

/* Moves to begin of last line. */
void win_mv_to_end_of_file(Win *);

/* Moves to begin of current line. */
void win_mv_to_end_of_line(Win *);

/* Moves to next word. */
void win_mv_to_next_word(Win *, size_t);

/* Moves to previous word. */
void win_mv_to_prev_word(Win *, size_t);

/* Moves up several times. */
void win_mv_up(Win *, size_t);

/* Opens terminal window with file. Do not forget to close it. */
Win *win_open(const char *, int, int);

/* Searches backward with passed query. */
void win_search_bwd(Win *, const char *);

/* Searches with passed query in passed direction. */
void win_search(Win *, const char *, enum Dir);

/* Saves opened file. Returns saved bytes count. */
size_t win_save_file(Win *);

/*
Saves opened file to spare directory. Returns saved bytes count. Writes path to
passed buffer.
*/
size_t win_save_file_to_spare_dir(Win *, char *, size_t);

/* Gets size of window. */
struct winsize win_size(const Win *);

/* Updates size of opened window using terminal. */
void win_upd_size(Win *);

#endif /* WIN_H */
