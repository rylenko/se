#include <stdlib.h>
#include <unistd.h>
#include "ed.h"
#include "err.h"
#include "term.h"

#define USAGE ("Usage:\n\t$ ew <filename>")

int
main(const int argc, const char *const *const argv)
{
	/* Check arguments count */
	if (argc != 2) {
		err(USAGE);
	}

	/* Initialize terminal */
	term_init(STDIN_FILENO, STDOUT_FILENO);
	/* Try open editor with file */
	ed_open(argv[1]);
	/* Enable terminal's raw mode */
	term_enable_raw_mode();

	/* Refresh editor's screen and process key presses */
	while (1) {
		ed_refresh_scr();
		/* This is needed here to clear the screen after quit in the key processor */
		if (ed_need_to_quit()) {
			break;
		}
		ed_wait_and_proc_key();
	}

	/* Disable raw mode, free buffer and exit */
	term_disable_raw_mode();
	return EXIT_SUCCESS;
}
