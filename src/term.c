#include <termios.h>
#include <unistd.h>
#include "res.h"
#include "term.h"

/* 1 is 100ms */
#define TIMEOUT_TENS_OF_SECOND 1

Res
term_disable_raw_mode(const struct termios* orig_termios)
{
	/* Restore original termios parameters */
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios) < 0)
		return RES_ERR_SET_ORIG_TERMIOS;
	return RES_OK;
}

Res
term_enable_raw_mode(struct termios* orig)
{
	struct termios raw;

	/* Check stdin refers to terminal */
	if (!isatty(STDIN_FILENO))
		return RES_STDIN_NOT_REF_TO_TERM;

	/* Get the original termios parameters */
	if (tcgetattr(STDIN_FILENO, orig) < 0)
		return RES_ERR_GET_ORIG_TERMIOS;

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
		return RES_ERR_SET_RAW_TERMIOS;
	return RES_OK;
}
