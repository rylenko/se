#include <stdlib.h>
#include <unistd.h>
#include "buf.h"
#include "editor.h"
#include "err.h"
#include "term.h"

#define USAGE ("Usage:\n\t$ vega <filename>")

int
main(const int argc, const char *const *const argv)
{
	Editor editor;

	/* Check arguments count */
	if (argc != 2)
		err(USAGE);
	/* Initialize terminal and enable raw mode */
	term_init(STDIN_FILENO, STDOUT_FILENO);
	term_enable_raw_mode();

	/* Open editor with file */
	editor = editor_new(argv[1]);
	/* Main cycle */
	while (1) {
		/* Refresh editor's screen */
		editor_refresh_scr(&editor);
	}

	/* Disable raw mode, free buffer and exit */
	term_disable_raw_mode();
	return EXIT_SUCCESS;
}
