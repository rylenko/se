#include <stdio.h>
#include <stdlib.h>

#include "term.h"

#define USAGE "Usage:\n\t$ vega <filename>"

/* Return codes */
typedef enum {
	CODE_SUCCESS = EXIT_SUCCESS,
	CODE_INVALID_ARGS_CNT = 1,
} Code;

int
main(const int argc, const char *const *const argv)
{
	/* Check arguments count */
	if (argc != 2) {
		puts(USAGE);
		return CODE_INVALID_ARGS_CNT;
	}

	/* Initialize terminal */
	printf("Filename: %s\n", argv[1]);
	term_init();

	return CODE_SUCCESS;
}
