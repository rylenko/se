#ifndef _WIN_H
#define _WIN_H

#include <stddef.h>
#include <sys/ioctl.h>
#include "vec.h"

/*
 * Opaque struct with window parameters.
 */
struct win;

/*
 * Closes the window.
 *
 * Returns 0 on success and -1 on error.
 */
int win_close(struct win *);

/*
 * Gets current line's index.
 */
size_t win_curr_line_idx(const struct win *);

/*
 * Gets current line char's index.
 */
size_t win_curr_line_char_idx(const struct win *);

/*
 * Breaks current line at cursor position.
 */
int win_break_line(struct win *);

/*
 * Deletes character before the cursor.
 */
int win_del_char(struct win *);

/*
 * Deletes the passed number of lines starting from the current one.
 *
 * Returns 0 on success and -1 if there is only one line which cannot be
 * deleted.
 *
 * Sets `ENOSYS` if there is one last line left that cannot be deleted.
 */
int win_del_line(struct win *, size_t);

/*
 * Draws cursor.
 */
int win_draw_cur(const struct win *, struct vec *);

/*
 * Draws window rows.
 */
int win_draw_lines(const struct win *, struct vec *);

/*
 * Checks that opened file is dirty.
 */
char win_file_is_dirty(const struct win *);

/*
 * Returns opened file's path.
 */
const char *win_file_path(const struct win *);

/*
 * Inserts character to the file.
 */
int win_ins_char(struct win *, char);

/*
 * Inserts empty line below several times.
 */
int win_ins_empty_line_below(struct win *, size_t);

/*
 * Inserts empty line on top of cursor several times.
 */
int win_ins_empty_line_on_top(struct win *, size_t);

/*
 * Move down several times.
 */
int win_mv_down(struct win *, size_t);

/*
 * Move left several times.
 */
int win_mv_left(struct win *, size_t);

/*
 * Move right several times.
 */
int win_mv_right(struct win *, size_t);

/*
 * Moves to begin of first line.
 */
void win_mv_to_begin_of_file(struct win *);

/*
 * Moves to begin of current line.
 */
void win_mv_to_begin_of_line(struct win *);

/*
 * Moves to begin of last line.
 */
void win_mv_to_end_of_file(struct win *);

/*
 * Moves to begin of current line.
 */
int win_mv_to_end_of_line(struct win *);

/*
 * Moves to next word.
 */
int win_mv_to_next_word(struct win *, size_t);

/*
 * Moves to previous word.
 */
int win_mv_to_prev_word(struct win *, size_t);

/*
 * Moves up several times.
 */
int win_mv_up(struct win *, size_t);

/*
 * Opens terminal window with file. Do not forget to close it.
 */
struct win *win_open(const char *, int, int);

/*
 * Saves opened file. Returns saved bytes count.
 */
size_t win_save_file(struct win *);

/*
 * Saves opened file to spare directory. Returns saved bytes count. Writes path
 * to passed buffer.
 */
size_t win_save_file_to_spare_dir(struct win *, char *, size_t);

/*
 * Searches backward from current position to end of file using passed query.
 *
 * Returns 0 on success and -1 on error.
 */
int win_search_bwd(struct win *, const char *);

/*
 * Searches forward from current position to end of file using passed query.
 *
 * Returns 0 on success and -1 on error.
 */
int win_search_fwd(struct win *, const char *);

/*
 * Gets size of window.
 */
struct winsize win_size(const struct win *);

/*
 * Updates size of opened window using terminal.
 */
int win_upd_size(struct win *);

#endif /* WIN_H */
