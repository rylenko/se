#include <string.h>
#include "path.h"

const char*
path_get_fname(const char *const path)
{
	const char *const last_sep = strrchr(path, '/');
	return NULL == last_sep ? path : &last_sep[1];
}
