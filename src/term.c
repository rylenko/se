#include <termios.h>
#include <unistd.h>
#include "err.h"
#include "term.h"

/* 1 is 100ms */
#define TIMEOUT_TENS_OF_SECOND 1

void
term_disable_raw_mode(const struct termios* orig)
{
	/* Restore original termios parameters */
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, orig) < 0)
		err("Failed to restore original termios parameters:");
}

void
term_enable_raw_mode(struct termios* orig)
{
	struct termios raw;

	/* Check stdin refers to terminal */
	if (!isatty(STDIN_FILENO))
		err("stdin does not refer to the terminal.");
	/* Get the original termios parameters */
	else if (tcgetattr(STDIN_FILENO, orig) < 0)
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
	/* Timeout. 1 is 100ms */
	raw.c_cc[VTIME] = TIMEOUT_TENS_OF_SECOND;

	/* Set new parameters */
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0)
		err("Failed to set raw termios parameters:");
}
