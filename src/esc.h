#ifndef _ESC_H
#define _ESC_H

#include "buf.h"
#include "color.h"

enum ArrowKey {
	ARROW_KEY_UP = 'A',
	ARROW_KEY_DOWN = 'B',
	ARROW_KEY_RIGHT = 'C',
	ARROW_KEY_LEFT = 'D',
};

/* Disables alternate screen. Need to restore screen before editor opening. */
void esc_alt_scr_off(void);

/* Enables alternate screen. Need to save screen before editor opening. */
void esc_alt_scr_on(void);

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

/* Gets escape sequence is arrow key. Returns 0 on success, otherwise -1. */
int esc_get_arrow_key(const char *, const size_t, enum ArrowKey *);

/* Moves the current writing pointer to the beginning of the window. */
void esc_go_home(Buf *);

#endif /* _ESC_H */
