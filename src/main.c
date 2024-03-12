#include <stdlib.h>
#include <unistd.h>
#include "ed.h"
#include "err.h"

#define USAGE ("Usage:\n\t$ ew <filename>")

int
main(const int argc, const char *const *const argv)
{
	/* Check arguments count */
	if (argc != 2) {
		err(USAGE);
	}
	/* Initialize editor and open the file */
	ed_init(STDIN_FILENO, STDOUT_FILENO);
	ed_open(argv[1]);
	/* Refresh editor's screen and process key presses */
	while (1) {
		ed_refresh_scr();
		/* Need here to clear the screen during last screen refreshing */
		if (ed_need_to_quit()) {
			break;
		}
		ed_wait_and_proc_key();
	}
	/* Deiniailize editor and exit */
	ed_deinit();
	return EXIT_SUCCESS;
}
