#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"

#define EXIT_CODE (1)

void
err(const char *fmt, ...)
{
	va_list args;

	/* Collect variadic arguments and print the main message */
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	/* Print errno message if there is a ":" at the end */
	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(EXIT_CODE);
}
