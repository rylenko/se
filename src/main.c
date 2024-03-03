#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "buf.h"
#include "err.h"
#include "term.h"

#define IN_FD (STDIN_FILENO)
#define OUT_FD (STDOUT_FILENO)
#define USAGE ("Usage:\n\t$ vega <filename>")

int
main(const int argc, const char *const *const argv)
{
	/* Variables */
	Buf buf = buf_alloc();
	struct termios orig_termios;

	/* Check arguments count */
	if (argc != 2)
		err(USAGE);
	/* Enable raw mode */
	term_enable_raw_mode(IN_FD, &orig_termios);

	buf_writef(&buf, "Filename: %s\r\n", argv[1]);
	buf_write(&buf, "Hello, ", 7);
	buf_write(&buf, "world!\r\n", 8);
	buf_writef(&buf, "Executable name: %s\r\n", argv[0]);
	buf_flush(&buf, OUT_FD);

	/* Disable raw mode, free buffer and exit */
	term_disable_raw_mode(IN_FD, &orig_termios);
	buf_free(buf);
	return EXIT_SUCCESS;
}
