#include <string.h>
#include "path.h"

const char*
path_get_fname(const char *const path)
{
	const char *last_sep;

	last_sep = strrchr(path, '/');
	return NULL == last_sep ? path : &last_sep[1];
}
