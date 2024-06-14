/* TODO: v0.4: perror errors in goto-cleanups */
/* TODO: v0.4: Create Cell struct to handle all symbols including UTF-8. Create structs Win->Renders->Render->Cells->Cell. Rerender lines on window side */
/* TODO: v0.4: Use linked list for lines array and line's content parts. */
/* TODO: v0.4: Remember last position per line. */
/* TODO: v0.5: Undo operations. Also rename "del" to "remove" where needed. */
/* TODO: v0.5: Add key settings for escape sequences. For example, CFG_KEY_MV_UP_2 = "..." */
/* TODO: v0.5: Add local clipboard. Use it in functions. */
/* TODO: v0.5: Xclip patch to use with local clipboard. */
/* TODO: v0.6: Support huge files: read chunks or try mmap */
/* TODO: v0.6: Add tests. */
/* TODO: v0.7: Make code patching easier. */
/* TODO: v0.7: Add more error codes in docs. */
/* TODO: v0.7: Save to spare dir on error. */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ed.h"

static const char *const usage = "Usage:\n\t$ se <filename>\n";

/*
 * Main loop of the program. Edits the file by passed filename.
 *
 * Returns `EXIT_SUCCESS` on success and `EXIT_FAILURE` on error.
 */
static int edit(const char *);

/*
 * Editor signals handler.
 */
static void handle_signal(int, siginfo_t *, void *);

/*
 * Setups signal handler for the editor. Must be called after editor opening.
 *
 * Returns 0 on success and -1 on error.
 */
static int setup_signal_handler(void);

/* Global editor variable, which used all the time. */
static struct ed *ed;

static int
edit(const char *const path)
{
	const char *err;
	int ret;

	/* Opens file in the editor. */
	ed = ed_open(path, STDIN_FILENO, STDOUT_FILENO);
	if (NULL == ed) {
		perror("Failed to open the editor");
		return EXIT_FAILURE;
	}

	/* Setup signal handler. */
	ret = setup_signal_handler();
	if (-1 == ret) {
		err = "Failed to setup a signal handler";
		goto err_quit;
	}

	/* Main event loop. */
	while (!ed_need_to_quit(ed)) {
		/* Draws editor's content on the screen. */
		ret = ed_draw(ed);
		if (-1 == ret) {
			err = "Failed to draw";
			goto err_quit;
		}
		/* Wait and process key presses. */
		ret = ed_wait_and_proc_key(ed);
		if (-1 == ret) {
			err = "Failed to wait and process key";
			goto err_quit;
		}
	}

	/* Quit the editor. */
	ret = ed_quit(ed);
	if (-1 == ret) {
		perror("Failed to quit");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
err_quit:
	/* Error checking here is useless. */
	ed_quit(ed);
	/* Print error after quit to disable raw mode properly. */
	perror(err);
	return EXIT_FAILURE;
}

static void
handle_signal(int signal, siginfo_t *info, void *ctx)
{
	(void)info;
	(void)ctx;
	if (NULL != ed)
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
	if (argc != 2) {
		fputs(usage, stderr);
		return EXIT_FAILURE;
	}

	/* Edit the file. */
	ret = edit(argv[1]);
	return ret;
}
