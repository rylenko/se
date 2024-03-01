#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include "res.h"
#include "term.h"

#define USAGE "Usage:\n\t$ vega <filename>\n"

int
main(const int argc, const char *const *const argv)
{
	Res res;
	struct termios orig_termios;

	/* Check arguments count */
	if (argc != 2) {
		puts(USAGE);
		res = RES_INVALID_ARGS_CNT;
		goto err;
	}

	/* Enable raw mode */
	if ((res = term_enable_raw_mode(&orig_termios)) != RES_OK)
		goto err;

	printf("Filename: %s\n", argv[1]);

	/* Disable raw mode */
	if ((res = term_disable_raw_mode(&orig_termios)) != RES_OK)
		goto err;

	return RES_OK;

err:
	res_print(res);
	return res;
}
