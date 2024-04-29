#ifndef _ESC_H
#define _ESC_H

#include "color.h"
#include "vec.h"

enum arrow_key {
	ARROW_KEY_UP = 'A',
	ARROW_KEY_DOWN = 'B',
	ARROW_KEY_RIGHT = 'C',
	ARROW_KEY_LEFT = 'D',
};

enum mouse_wh_key {
	MOUSE_WH_KEY_DOWN = 'a',
	MOUSE_WH_KEY_UP = '`',
};

/*
Disables alternate screen. Need to restore screen before editor opening.

Returns 0 on success and -1 on error.
*/
int esc_alt_scr_off(Vec *);

/*
Enables alternate screen. Need to save screen before editor opening. Do not
forget to disable it.

Returns 0 on success and -1 on error.
*/
int esc_alt_scr_on(Vec *);

/*
Clears all window.

Returns 0 on success and -1 on error.
*/
int esc_clr_win(Vec *);

/*
Begins colored background.

Returns 0 on success and -1 on error.
*/
int esc_color_bg(Vec *, struct color);

/*
Begins colored foreground.

Returns 0 on success and -1 on error.
*/
int esc_color_fg(Vec *, struct color);

/*
Ends colored output.

Returns 0 on success and -1 on error.
*/
int esc_color_end(Vec *);

/*
Hides the cursor. Used to avoid blinking during redrawing.

Returns 0 on success and -1 on error.
*/
int esc_cur_hide(Vec *);

/*
Sets the cursor in the window. Values start from zero.

Returns 0 on success and -1 on error.
*/
int esc_cur_set(Vec *, unsigned short, unsigned short);

/*
Shows the cursor.

Returns 0 on success and -1 on error.
*/
int esc_cur_show(Vec *);

/*
Extracts arrow key from sequence.

Returns 0 on success and -1 on error.

Sets no errors.
*/
int esc_extr_arrow_key(const char *, size_t, enum arrow_key *);

/*
Extracts mouse wheel key from sequence.

Returns 0 on success and -1 on error.

Sets no errors.
*/
int esc_extr_mouse_wh_key(const char *, size_t, enum mouse_wh_key *);

/*
Moves the current writing pointer to the beginning of the window.

Returns 0 on success and -1 on error.
*/
int esc_go_home(Vec *);

/*
Disables mouse wheel tracking.

Returns 0 on success and -1 on error.
*/
int esc_mouse_wh_track_off(Vec *);

/*
Enables mouse wheel tracking. Do not forget to disable it.

Returns 0 on success and -1 on error.
*/
int esc_mouse_wh_track_on(Vec *);

#endif /* _ESC_H */
