#include "buf.h"
#include "cur.h"

void
cur_hide(Buf *const buf)
{
	buf_write(buf, "\x1b[?25l", 6);
}

Cur
cur_new(unsigned short x, unsigned short y)
{
	return (Cur){ .x = x, .y = y };
}

void
cur_show(Buf *const buf)
{
	buf_write(buf, "\x1b[?25h", 6);
}

void
cur_write(const Cur cur, Buf *const buf)
{
	buf_writef(buf, "\x1b[%hu;%huH", cur.y + 1, cur.x + 1);
}
