#include "pos.h"

void
pos_init(Pos *const pos)
{
	/* Set default offset */
	pos->offset.x = 0;
	pos->offset.y = 0;
	/* Initialize cursor */
	cur_init(&pos->cur);
}
