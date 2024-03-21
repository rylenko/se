/* TODO: [1] fix offset shifting with tabs in the window. See `Pos` todo  */
/* TODO: [1] Create `Pos` struct to store file position and window position */
/* TODO: [1] Spend time on refactoring and arch after [1] todos */
/* TODO: [2] Add local clipboard. Use it in functions. */
/* TODO: [2] Use linked list for rows array and row's content parts. Benchmark! */
/* TODO: [2] Undo operations. Also rename "del" to "remove" where needed */
/* TODO: [3] Xclip patch to use with local clipboard */
/* TODO: [3] Support huge files */

#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include "ed.h"
#include "ed_draw.h"
#include "ed_term.h"
#include "ed_key.h"
#include "ed_quit.h"

static const char *const usage = "Usage:\n\t$ se <filename>";

int
main(const int argc, const char *const *const argv)
{
	Ed ed;
	/* Check arguments count */
	if (argc != 2) {
		errx(EXIT_FAILURE, usage);
	}
	/* Open the file and initialize editor */
	ed = ed_open(argv[1]);
	ed_term_init(&ed, STDIN_FILENO, STDOUT_FILENO);
	/* Refresh editor's window and process key presses */
	while (1) {
		ed_draw(&ed);
		/* Need here to clear the window during last window refreshing */
		if (ed_quit_done(&ed)) {
			break;
		}
		ed_key_wait_and_proc(&ed);
	}
	/* Deiniailize editor and exit */
	ed_term_deinit();
	return EXIT_SUCCESS;
}
