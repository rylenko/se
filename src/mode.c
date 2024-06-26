#include <stddef.h>
#include "mode.h"

char*
mode_str(const enum mode mode)
{
	switch (mode) {
	case MODE_INS:
		return "INSERT";
	case MODE_NORM:
		return "NORMAL";
	case MODE_SEARCH:
		return "SEARCH";
	default:
		return "UNKNOWN";
	}
}
