#include "buf.h"
#include "color.h"
#include "esc.h"
#include "term.h"

void
esc_alt_scr_on(void)
{
	term_write("\x1b[?1049h", 8);
}

void
esc_alt_scr_off(void)
{
	term_write("\x1b[?1049l", 8);
}

void
esc_clr_win(Buf *const buf)
{
	buf_write(buf, "\x1b[2J", 4);
}

void
esc_color_begin(
	Buf *const buf,
	const struct Color *const fg,
	const struct Color *const bg
) {
	/* Write foreground if set */
	if (fg != NULL)
		buf_writef(buf, "\x1b[38;2;%hhu;%hhu;%hhum", fg->r, fg->g, fg->b);
	/* Write background if set */
	if (bg != NULL)
		buf_writef(buf, "\x1b[48;2;%hhu;%hhu;%hhum", bg->r, bg->g, bg->b);
}

void
esc_color_end(Buf *const buf)
{
	buf_write(buf, "\x1b[0m", 4);
}

void
esc_cur_hide(Buf *const buf)
{
	buf_write(buf, "\x1b[?25l", 6);
}

void
esc_cur_set(Buf *const buf, const unsigned short row, const unsigned short col)
{
	buf_writef(buf, "\x1b[%hu;%huH", row + 1, col + 1);
}

void
esc_cur_show(Buf *const buf)
{
	buf_write(buf, "\x1b[?25h", 6);
}

int
esc_get_arrow_key(
	const char *const seq,
	const size_t len,
	enum ArrowKey *const key
) {
	if (
		3 == len
		&& '\x1b' == seq[0]
		&& '[' == seq[1]
		&& ARROW_KEY_UP <= seq[2]
		&& seq[2] <= ARROW_KEY_LEFT
	) {
		*key = seq[2];
		return 0;
	}
	return -1;
}

void
esc_go_home(Buf *const buf)
{
	buf_write(buf, "\x1b[H", 3);
}
