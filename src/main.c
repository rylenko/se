#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include "err.h"
#include "term.h"

#define USAGE ("Usage:\n\t$ vega <filename>")

int
main(const int argc, const char *const *const argv)
{
	struct termios orig_termios;

	/* Check arguments count */
	if (argc != 2)
		err(USAGE);

	/* Enable raw mode */
	term_enable_raw_mode(&orig_termios);

	printf("Filename: %s\n", argv[1]);

	/* Disable raw mode and exit */
	term_disable_raw_mode(&orig_termios);
	return EXIT_SUCCESS;
}
