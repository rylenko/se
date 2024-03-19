#include "buf.h"
#include "raw_color.h"

enum {
	BG_OFFSET = 10,
};

void
raw_color_begin(Buf *const buf, const RawColor bg, const RawColor fg)
{
	buf_writef(buf, "\x1b[%d;%dm", fg, bg + BG_OFFSET);
}

void
raw_color_end(Buf *const buf)
{
	buf_write(buf, "\x1b[0m", 4);
}
