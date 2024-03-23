#include <string.h>
#include "pos.h"

void
pos_init(Pos *const pos)
{
	/* Zeroize instance */
	memset(pos, 0, sizeof(*pos));
	/* Initialize cursor */
	cur_init(&pos->cur);
}
