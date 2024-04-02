/* TODO: [1] Remove unused includes. */
/* TODO: [1] Try to make code easier and less bloated. */
/* TODO: [1] Add more clear docs and comments. */
/* TODO: [1] Handle non-printable characters. */
/* TODO: [2] Add local clipboard. Use it in functions. */
/* TODO: [2] Xclip patch to use with local clipboard. */
/* TODO: [2] Use linked list for lines array and line's content parts. */
/* TODO: [3] Support huge files. */
/* TODO: [3] Undo operations. Also rename "del" to "remove" where needed. */
/* TODO: [4] API with status codes instead of err.h and tests */

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "ed.h"

static const char *const usage = "Usage:\n\t$ se <filename>";

/* Handle editor signals. */
static void handle_signal(int signal);

static Ed *ed;

static void
handle_signal(int signal)
{
	ed_handle_signal(ed, signal);
}

int
main(const int argc, const char *const *const argv)
{
	/* Check filename in arguments */
	if (argc != 2)
		errx(EXIT_FAILURE, usage);

	/* Opens file in the editor */
	ed = ed_open(argv[1], STDIN_FILENO, STDOUT_FILENO);

	/* Register signals handler */
	if (signal(SIGWINCH, handle_signal) == SIG_ERR)
		err(EXIT_FAILURE, "Failed to set signal handler.");

	while (1) {
		/* Draws editor's content on the screen */
		ed_draw(ed);
		/* Check that we need to quit. Need here to clear screen before quit */
		if (ed_need_to_quit(ed))
			break;
		/* Wait and process key presses */
		ed_wait_and_proc_key(ed);
	}

	/* Quit the editor */
	ed_quit(ed);
	return EXIT_SUCCESS;
}
