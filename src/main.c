/* TODO: [1] Add local clipboard. Use it in functions. */
/* TODO: [1] Use linked list for rows array and row's content parts. Benchmark! */
/* TODO: [1] Undo operations. Also rename "del" to "remove" where needed */
/* TODO: [2] Xclip patch to use with local clipboard */
/* TODO: [2] Support huge files */

#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include "ed.h"
#include "ed_draw.h"
#include "ed_key.h"

static const char *const usage = "Usage:\n\t$ se <filename>";

int
main(const int argc, const char *const *const argv)
{
	Ed ed;

	/* Check filename in arguments */
	if (argc != 2)
		errx(EXIT_FAILURE, usage);

	/* Opens file in the editor */
	ed_open(&ed, argv[1], STDIN_FILENO, STDOUT_FILENO);

	while (1) {
		/* Draws editor's content on the screen */
		ed_draw(&ed);
		/* Check that we need to quit. Need here to clear screen before quit */
		if (ed_need_to_quit(&ed))
			break;
		/* Wait and process key presses */
		ed_key_wait_and_proc(&ed);
	}

	/* Quit the editor */
	ed_quit(&ed);
	return EXIT_SUCCESS;
}
