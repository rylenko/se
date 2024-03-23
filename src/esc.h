#ifndef _ESC_H
#define _ESC_H

#include "buf.h"
#include "cur.h"

/* Clears the row on the right. */
void esc_clr_right(Buf *);

/* Clears all window. */
void esc_clr_win(Buf *);

/* Sets 256-color foreground and background. */
void esc_color_begin(Buf *, unsigned char, unsigned char);

/* Ends colored output. */
void esc_color_end(Buf *);

/* Hides the cursor. Used to avoid blinking during redrawing. */
void esc_cur_hide(Buf *);

/* Sets the cursor in the window. */
void esc_cur_set(Buf *, const Cur *);

/* Shows the cursor. */
void esc_cur_show(Buf *);

/* Moves the current writing pointer to the beginning of the window. */
void esc_go_home(Buf *);

#endif /* _ESC_H */
