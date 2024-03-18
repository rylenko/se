#include <stdlib.h>
#include "mode.h"

char*
mode_str(const Mode mode)
{
	return mode == MODE_INS ? "INSERT" : (mode == MODE_NORM ? "NORMAL" : NULL);
}
