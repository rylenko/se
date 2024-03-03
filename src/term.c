#include <termios.h>
#include "err.h"
#include "term.h"

#define TIMEOUT_TENTHS_OF_SECOND 1

void
term_disable_raw_mode(int fd, const struct termios* orig)
{
	/* Restore original termios parameters */
	if (tcsetattr(fd, TCSAFLUSH, orig) < 0)
		err("Failed to restore original termios parameters:");
}

void
term_enable_raw_mode(int fd, struct termios* orig)
{
	struct termios raw;

	/* Get the original termios parameters */
	if (tcgetattr(fd, orig) < 0)
		err("Failed to get original termios parameters:");

	/* Not `NULL` because we got it earlier */
	raw = *orig;
	/*
	Input: no break, no CR to NL, no parity check, no char stripping, no start
	and stop of output control
	*/
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	/* Output: no post processing */
	raw.c_oflag &= ~OPOST;
	/* Control: 8-bit chars */
	raw.c_cflag |= CS8;
	/* Local: no echo, no canonical, no extended functions, no signals */
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	/* Return zero byte on input timeout */
	raw.c_cc[VMIN] = 0;
	/* Timeout in tenths of a second */
	raw.c_cc[VTIME] = TIMEOUT_TENTHS_OF_SECOND;

	/* Set new parameters */
	if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
		err("Failed to set raw termios parameters:");
}
