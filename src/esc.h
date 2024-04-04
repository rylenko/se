#ifndef _ESC_H
#define _ESC_H

#include "buf.h"
#include "color.h"

/* Clears all window. */
void esc_clr_win(Buf *);

/* Sets colored foreground and background. `NULL` if no color. */
void esc_color_begin(Buf *, const struct Color *, const struct Color *);

/* Ends colored output. */
void esc_color_end(Buf *);

/* Hides the cursor. Used to avoid blinking during redrawing. */
void esc_cur_hide(Buf *);

/* Sets the cursor in the window. Values start from zero. */
void esc_cur_set(Buf *, unsigned short, unsigned short);

/* Shows the cursor. */
void esc_cur_show(Buf *);

/* Moves the current writing pointer to the beginning of the window. */
void esc_go_home(Buf *);

#endif /* _ESC_H */
