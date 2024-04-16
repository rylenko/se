#include <stdio.h>
#include <string.h>
#include "color.h"
#include "esc.h"
#include "vec.h"

void
esc_alt_scr_on(Vec *const buf)
{
	vec_append(buf, "\x1b[?1049h", 8);
}

void
esc_alt_scr_off(Vec *const buf)
{
	vec_append(buf, "\x1b[?1049l", 8);
}

void
esc_clr_win(Vec *const buf)
{
	vec_append(buf, "\x1b[2J", 4);
}

void
esc_color_begin(
	Vec *const buf,
	const struct Color *const fg,
	const struct Color *const bg
) {
	char s[20];
	size_t len;

	/* Write foreground if set */
	if (fg != NULL) {
		len = \
			snprintf(s, sizeof(s), "\x1b[38;2;%hhu;%hhu;%hhum", fg->r, fg->g, fg->b);
		vec_append(buf, s, len);
	}
	/* Write background if set */
	if (bg != NULL) {
		len = \
			snprintf(s, sizeof(s), "\x1b[48;2;%hhu;%hhu;%hhum", bg->r, bg->g, bg->b);
		vec_append(buf, s, len);
	}
}

void
esc_color_end(Vec *const buf)
{
	vec_append(buf, "\x1b[0m", 4);
}

void
esc_cur_hide(Vec *const buf)
{
	vec_append(buf, "\x1b[?25l", 6);
}

void
esc_cur_set(Vec *const buf, const unsigned short row, const unsigned short col)
{
	char seq[15];
	size_t len = snprintf(seq, sizeof(seq), "\x1b[%hu;%huH", row + 1, col + 1);
	vec_append(buf, seq, len);
}

void
esc_cur_show(Vec *const buf)
{
	vec_append(buf, "\x1b[?25h", 6);
}

int
esc_extr_arrow_key(
	const char *const seq,
	const size_t len,
	enum ArrowKey *const key
) {
	if (
		3 == len
		&& strncmp("\x1b[", seq, 2) == 0
		&& ARROW_KEY_UP <= seq[2]
		&& seq[2] <= ARROW_KEY_LEFT
	) {
		*key = seq[2];
		return 0;
	}
	return -1;
}

int
esc_extr_mouse_wheel_key(
	const char *const seq,
	const size_t len,
	enum MouseWheelKey *const key
) {
	if (
		4 == len
		&& strncmp("\x1b[M", seq, 3) == 0
		&& MOUSE_WHEEL_KEY_UP <= seq[3]
		&& seq[3] <= MOUSE_WHEEL_KEY_DOWN
	) {
		*key = seq[3];
		return 0;
	}
	return -1;
}

void
esc_go_home(Vec *const buf)
{
	vec_append(buf, "\x1b[H", 3);
}

void
esc_mouse_wheel_track_off(Vec *const buf)
{
	vec_append(buf, "\x1b[?1000l", 8);
}

void
esc_mouse_wheel_track_on(Vec *const buf)
{
	vec_append(buf, "\x1b[?1000h", 8);
}
