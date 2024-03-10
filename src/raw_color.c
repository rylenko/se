#include "buf.h"
#include "raw_color.h"

#define BG_OFFSET (10)

void
raw_color_begin(Buf *buf, RawColor bg, RawColor fg)
{
	buf_writef(buf, "\x1b[%d;%dm", fg, bg + BG_OFFSET);
}

void
raw_color_end(Buf *buf)
{
	buf_write(buf, "\x1b[0m", 4);
}
