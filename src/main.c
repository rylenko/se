/* TODO: [1] Use ptrdiff_t where needed. */
/* TODO: [1] Try to make code easier and less bloated. */
/* TODO: [1] Add more clear docs and comments. */
/* TODO: [1] Add local clipboard. Use it in functions. */
/* TODO: [1] Use linked list for lines array and line's content parts. */
/* TODO: [1] Undo operations. Also rename "del" to "remove" where needed. */
/* TODO: [2] Xclip patch to use with local clipboard. */
/* TODO: [2] Support huge files. */

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "ed.h"
#include "ed_draw.h"
#include "ed_key.h"

static const char *const usage = "Usage:\n\t$ se <filename>";

/* Handle editor signals. */
static void handle_signal(int signal);

static Ed ed;

static void
handle_signal(int signal)
{
	ed_draw_handle_signal(&ed, signal);
}

int
main(const int argc, const char *const *const argv)
{
	/* Check filename in arguments */
	if (argc != 2)
		errx(EXIT_FAILURE, usage);

	/* Opens file in the editor */
	ed_open(&ed, argv[1], STDIN_FILENO, STDOUT_FILENO);

	/* Register signals handler */
	if (signal(SIGWINCH, handle_signal) == SIG_ERR)
		err(EXIT_FAILURE, "Failed to set signal handler.");

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
