#include <string.h>
#include "cur.h"

void
cur_init(Cur *const cur)
{
	memset(cur, 0, sizeof(*cur));
}
