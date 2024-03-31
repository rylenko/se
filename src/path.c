#include <string.h>
#include "path.h"

const char*
path_get_filename(const char *const path)
{
	const char *const filename_with_slash = strrchr(path, '/');
	return NULL == filename_with_slash ? path : filename_with_slash + 1;
}
