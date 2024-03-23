#include <stddef.h>
#include "mode.h"

char*
mode_str(const Mode mode)
{
	switch (mode) {
	case MODE_INS:
		return "INSERT";
	case MODE_NORM:
		return "NORMAL";
	default:
		return NULL;
	}
}
