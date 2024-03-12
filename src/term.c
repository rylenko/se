#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include "buf.h"
#include "err.h"
#include "term.h"

#define TIMEOUT_TENTHS_OF_SECOND (1)

static struct {
	int ifd;
	int ofd;
	struct termios orig;
} term;

void
term_clr_row_on_right(Buf *buf)
{
	buf_write(buf, "\x1b[0K", 4);
}

void
term_clr_scr(Buf *buf)
{
	buf_write(buf, "\x1b[2J", 4);
}

void
term_disable_raw_mode(void)
{
	/* Restore original termios parameters */
	if (tcsetattr(term.ifd, TCSAFLUSH, &term.orig) < 0) {
		err("Failed to restore original termios parameters:");
	}
}

void
term_enable_raw_mode(void)
{
	struct termios raw;

	/* Get the original termios parameters */
	if (tcgetattr(term.ifd, &term.orig) < 0) {
		err("Failed to get original termios parameters:");
	}
	/* Disable raw mode at exit */
	else if (atexit(term_disable_raw_mode) < 0) {
		err("Failed to set raw mode's disabler at exit.");
	}

	/* Not `NULL` because we got it earlier */
	raw = term.orig;
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
	if (tcsetattr(term.ifd, TCSAFLUSH, &raw) < 0) {
		err("Failed to set raw termios parameters:");
	}
}

void
term_flush(const Buf *buf)
{
	buf_flush(buf, term.ofd);
}

void
term_get_win_size(struct winsize *win_size)
{
	if (ioctl(term.ofd, TIOCGWINSZ, win_size) < 0) {
		err("Failed to get window size for fd %d:", term.ofd);
	}
}

void
term_go_home(Buf *buf)
{
	buf_write(buf, "\x1b[H", 3);
}

void
term_init(const int ifd, const int ofd)
{
	term.ifd = ifd;
	term.ofd = ofd;
}

char
term_wait_key(void)
{
	char key;
	int readed_cnt;

	/* Wait a key or an error */
	while (0 == (readed_cnt = read(term.ifd, &key, 1)));
	if (readed_cnt < 0) {
		err("Failed to read key press:");
	}
	return key;
}
