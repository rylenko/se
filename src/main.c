/* TODO: v0.2: Fix end of line with many tabs. */
/* TODO: v0.2: Add github action with `make gen-readme` */
/* TODO: v0.3: Create Cell struct to handle all symbols including UTF-8. Create structs Win->Renders->Render->Cells->Cell. Rerender lines on window side */
/* TODO: v0.3: Use linked list for lines array and line's content parts. */
/* TODO: v0.3: Undo operations. Also rename "del" to "remove" where needed. */
/* TODO: v0.3: Remember last position per line. */
/* TODO: v0.4: Add key settings for escape sequences. For example, CFG_KEY_MV_UP_2 = "..." */
/* TODO: v0.4: Add local clipboard. Use it in functions. */
/* TODO: v0.4: Xclip patch to use with local clipboard. */
/* TODO: v0.5: Add more clear docs and comments. */
/* TODO: v0.5: Support huge files. */
/* TODO: v0.6: API with status codes instead of err.h. Add tests */

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ed.h"

static const char *const usage = "Usage:\n\t$ se <filename>";

/* Handle editor signals. */
static void handle_signal(int signal, siginfo_t *info, void *ctx);

static Ed *ed;

static void
handle_signal(int signal, siginfo_t *info, void *ctx)
{
	(void)info;
	(void)ctx;
	ed_handle_signal(ed, signal);
}

static void
setup_signal_handler(void)
{
	/* Initialize action */
	struct sigaction action;
	memset(&action, 0, sizeof(action));

	/* Fill with all signals */
	sigfillset(&action.sa_mask);

	/* Set handler */
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = handle_signal;

	/* Register signals */
	if (sigaction(SIGWINCH, &action, NULL) == -1)
		err(EXIT_FAILURE, "Failed to register window size change signal");
}

int
main(const int argc, const char *const *const argv)
{
	/* Check filename in arguments */
	if (argc != 2)
		errx(EXIT_FAILURE, usage);

	/* Opens file in the editor and setup signal handler */
	ed = ed_open(argv[1], STDIN_FILENO, STDOUT_FILENO);
	setup_signal_handler();

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
