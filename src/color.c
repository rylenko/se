#include "buf.h"
#include "color.h"

void
color_begin(Buf *buf, Color bg, Color fg)
{
	buf_writef(buf, "\x1b[%d;%dm", fg, bg + 10);
}

void
color_end(Buf *buf)
{
	buf_write(buf, "\x1b[0m", 4);
}
