#ifndef _ESC_H
#define _ESC_H

#include "color.h"
#include "vec.h"

enum ArrowKey {
	ARROW_KEY_UP = 'A',
	ARROW_KEY_DOWN = 'B',
	ARROW_KEY_RIGHT = 'C',
	ARROW_KEY_LEFT = 'D',
};

enum MouseWheelKey {
	MOUSE_WHEEL_KEY_DOWN = 'a',
	MOUSE_WHEEL_KEY_UP = '`',
};

/* Disables alternate screen. Need to restore screen before editor opening. */
void esc_alt_scr_off(Vec *);

/*
Enables alternate screen. Need to save screen before editor opening. Do not
forget to disable it.
*/
void esc_alt_scr_on(Vec *);

/* Clears all window. */
void esc_clr_win(Vec *);

/* Sets colored foreground and background. `NULL` if no color. */
void esc_color_begin(Vec *, const struct Color *, const struct Color *);

/* Ends colored output. */
void esc_color_end(Vec *);

/* Hides the cursor. Used to avoid blinking during redrawing. */
void esc_cur_hide(Vec *);

/* Sets the cursor in the window. Values start from zero. */
void esc_cur_set(Vec *, unsigned short, unsigned short);

/* Shows the cursor. */
void esc_cur_show(Vec *);

/* Extracts arrow key from sequence. Returns 0 on success, otherwise -1. */
int esc_extr_arrow_key(const char *, size_t, enum ArrowKey *);

/*
Extracts mouse wheel key from sequence. Returns 0 on success, otherwise -1.
*/
int esc_extr_mouse_wheel_key(const char *, size_t, enum MouseWheelKey *);

/* Moves the current writing pointer to the beginning of the window. */
void esc_go_home(Vec *);

/* Disables mouse wheel tracking. */
void esc_mouse_wheel_track_off(Vec *);

/* Enables mouse wheel tracking. Do not forget to disable it. */
void esc_mouse_wheel_track_on(Vec *);

#endif /* _ESC_H */
