#include "buf.h"
#include "cur.h"
#include "esc.h"

void
esc_clr_right(Buf *const buf)
{
	buf_write(buf, "\x1b[0K", 4);
}

void
esc_clr_win(Buf *const buf)
{
	buf_write(buf, "\x1b[2J", 4);
}

void
esc_color_begin(Buf *const buf, unsigned char fd, unsigned char bg)
{
	/* Write foreground and background to buffer */
	buf_writef(buf, "\x1b[38;5;%hhu" "\x1b[48;5;%hhu", fg, bg);
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
esc_cur_set(Buf *const buf, const Cur *const cur)
{
	buf_writef(buf, "\x1b[%hu;%huH", cur->y + 1, cur->x + 1);
}

void
esc_cur_show(Buf *const buf)
{
	buf_write(buf, "\x1b[?25h", 6);
}

void
esc_go_home(Buf *const buf)
{
	buf_write(buf, "\x1b[H", 3);
}
