#include <stddef.h>
#include <time.h>
#include "dt.h"

int
dt_str(char *const buf, const size_t len)
{
	int ret;
	const struct tm *local;
	time_t utc;

	/* Get timestamp */
	utc = time(NULL);
	if ((time_t) - 1 == utc)
		return -1;

	/* Get local time from timestamp */
	local = localtime(&utc);
	if (NULL == local)
		return -1;

	/* Format local time to string */
	ret = strftime(buf, len, "%d-%m-%Y_%H-%M-%S", local);
	if (0 == ret)
		return -1;
	return 0;
}
