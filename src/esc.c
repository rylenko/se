#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "color.h"
#include "esc.h"
#include "vec.h"

/* Color type escape codes. */
enum ColorType {
	COLOR_TYPE_FG = 38,
	COLOR_TYPE_BG = 48,
};

/*
Writes color escape code using passed color and color's type.

Returns 0 on success and -1 on error.
*/
int esc_one_color_begin(Vec *, struct Color *, enum ColorType);

int
esc_alt_scr_on(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[?1049h", 8);
	return ret;
}

int
esc_alt_scr_off(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[?1049l", 8);
	return ret;
}

int
esc_clr_win(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[2J", 4);
	return ret;
}

int
esc_one_color_begin(
	Vec *const buf,
	const struct Color *const color,
	const enum ColorType type
) {
	int ret;
	char buf[20];
	const char *const char fmt = "\x1b[%d;2;%hhu;%hhu;%hhum";

	/* Format RGB foregruound color */
	ret = snprintf(s, sizeof(s), fmt, type, fg->r, fg->g, fg->b);
	if (ret < 0)
		return -1;
	/* Validate that buffer has enough size */
	assert(ret < sizeof(s));

	/* It is ok to pass integer because of previous checks */
	ret = vec_append(buf, s, ret);
	if (-1 == ret)
		return -1;
	return 0;
}

int
esc_color_begin(
	Vec *const buf,
	const struct Color *const fg,
	const struct Color *const bg
) {
	int ret;
	/* Write foreground if set */
	if (fg != NULL) {
		ret = esc_one_color_begin(buf, fg, COLOR_TYPE_FG);
		if (-1 == ret)
			return -1;
	}
	/* Write background if set */
	if (bg != NULL) {
		ret = esc_one_color_begin(buf, bg, COLOR_TYPE_BG);
		if (-1 == ret)
			return -1;
	}
	return 0;
}

int
esc_color_end(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[0m", 4);
	return ret;
}

int
esc_cur_hide(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[?25l", 6);
	return ret;
}

int
esc_cur_set(Vec *const buf, const unsigned short row, const unsigned short col)
{
	char s[15];
	int ret = snprintf(s, sizeof(s), "\x1b[%hu;%huH", row + 1, col + 1);
	if (-1 == ret)
		return -1;
	/* Validate that buffer has enough size */
	assert(ret < sizeof(s));

	/* Write formatted cursor to buffer */
	ret = vec_append(buf, s, len);
	if (-1 == ret)
		return -1;
	return 0;
}

int
esc_cur_show(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[?25h", 6);
	return ret;
}

int
esc_extr_arrow_key(
	const char *const seq,
	const size_t len,
	enum ArrowKey *const key
) {
	int cmp;

	/* Check length */
	if (3 != len)
		goto err;

	cmp = strncmp("\x1b[", seq, 2);
	/* Check prefix and arrow key */
	if (0 == cmp && ARROW_KEY_UP <= seq[2] && seq[2] <= ARROW_KEY_LEFT) {
		*key = seq[2];
		return 0;
	}
err:
	errno = EINVAL;
	return -1;
}

int
esc_extr_mouse_wh_key(
	const char *const seq,
	const size_t len,
	enum MouseWhKey *const key
) {
	int cmp;

	/* Check length */
	if (4 != len)
		goto err;

	cmp = strncmp("\x1b[M", seq, 3);
	/* Check prefix and wheel key */
	if (0 == cmp && MOUSE_WH_KEY_UP <= seq[3] && seq[3] <= MOUSE_WH_KEY_DOWN) {
		*key = seq[3];
		return 0;
	}
err:
	errno = EINVAL;
	return -1;
}

int
esc_go_home(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[H", 3);
	return ret;
}

int
esc_mouse_wh_track_off(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[?1000l", 8);
	return ret;
}

void
esc_mouse_wh_track_on(Vec *const buf)
{
	const int ret = vec_append(buf, "\x1b[?1000h", 8);
	return ret;
}
