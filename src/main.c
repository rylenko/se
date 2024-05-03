/* TODO: v0.3: Check using valgrind and eyes that all memory is freed on errors. */
/* TODO: v0.3: Split functions into smaller functions, which "do one thing and do it well". */
/* TODO: v0.4: Create Cell struct to handle all symbols including UTF-8. Create structs Win->Renders->Render->Cells->Cell. Rerender lines on window side */
/* TODO: v0.4: Use linked list for lines array and line's content parts. */
/* TODO: v0.4: Remember last position per line. */
/* TODO: v0.4: Rewrite moving functions with more unambiguous behavior and minimal scrolling. */
/* TODO: v0.5: Undo operations. Also rename "del" to "remove" where needed. */
/* TODO: v0.5: Add key settings for escape sequences. For example, CFG_KEY_MV_UP_2 = "..." */
/* TODO: v0.5: Add local clipboard. Use it in functions. */
/* TODO: v0.5: Xclip patch to use with local clipboard. */
/* TODO: v0.6: Support huge files: read chunks or try mmap */
/* TODO: v0.6: Add tests. */
/* TODO: v0.7: Make code patching easier. */
/* TODO: v0.7: Add more error codes in docs. */
/* TODO: v0.7: Save to spare dir on error. */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ed.h"

static const char *const usage = "Usage:\n\t$ se <filename>";

/*
 * Editor signals handler.
 */
static void handle_signal(int, siginfo_t *, void *);

/*
 * Setups signal handler for the editor.
 */
static int setup_signal_handler(void);

/* Global editor variable, which used all the time. */
static struct ed *ed;

static void
handle_signal(int signal, siginfo_t *info, void *ctx)
{
	(void)info;
	(void)ctx;
	ed_reg_sig(ed, signal);
}

static int
setup_signal_handler(void)
{
	int ret;
	struct sigaction action;

	/* Initialize action. */
	memset(&action, 0, sizeof(action));
	/* Fill with all signals. */
	ret = sigfillset(&action.sa_mask);
	if (-1 == ret)
		return -1;
	/* Set handler. */
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = handle_signal;

	/* Register signals. */
	ret = sigaction(SIGWINCH, &action, NULL);
	return ret;
}

int
main(const int argc, const char *const *const argv)
{
	int ret;

	/* Check filename in arguments. */
	if (argc != 2)
		errx(EXIT_FAILURE, usage);

	/* Setup signal handler. */
	ret = setup_signal_handler();
	if (-1 == ret)
		err(EXIT_FAILURE, "Failed to setup signal handler");

	/* Opens file in the editor. */
	ed = ed_open(argv[1], STDIN_FILENO, STDOUT_FILENO);
	if (NULL == ed)
		err(EXIT_FAILURE, "Failed to open the editor");

	/* Main event loop. */
	while (!ed_need_to_quit(ed)) {
		/* Draws editor's content on the screen. */
		ret = ed_draw(ed);
		if (-1 == ret)
			err(EXIT_FAILURE, "Failed to draw");

		/* Wait and process key presses. */
		ret = ed_wait_and_proc_key(ed);
		if (-1 == ret)
			err(EXIT_FAILURE, "Failed to wait and process key");
	}

	/* Quit the editor. */
	ret = ed_quit(ed);
	if (-1 == ret)
		err(EXIT_FAILURE, "Failed to quit the editor");
	return EXIT_SUCCESS;
}
