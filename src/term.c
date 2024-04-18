#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "term.h"

/* Structure for controlling input and output */
struct {
	int ifd; /* Input file descriptor. Usually stdin */
	int ofd; /* Output file descriptor. Usually stdout */
	struct termios orig_termios; /* Original termios without our changes */
} term;

/* Sets raw mode parameters to termios instance. */
static void term_set_raw_mode_params(struct termios *);

void
term_deinit(void)
{
	/* Restore original termios parameters to disable raw mode */
	if (tcsetattr(term.ifd, TCSANOW, &term.orig_termios) == -1)
		err(EXIT_FAILURE, "Failed to restore original termios parameters");
}

void
term_get_win_size(struct winsize *const win_size)
{
	if (ioctl(term.ofd, TIOCGWINSZ, win_size) == -1)
		err(EXIT_FAILURE, "Failed to get window size for fd %d", term.ofd);
}

void
term_init(const int ifd, const int ofd)
{
	struct termios raw_termios;

	/* Set file descriptors */
	term.ifd = ifd;
	term.ofd = ofd;

	/* Save the original termios parameters */
	if (tcgetattr(ifd, &term.orig_termios) == -1)
		err(EXIT_FAILURE, "Failed to get original termios");
	/* Set terminal deiniter on exit */
	if (atexit(term_deinit) != 0)
		errx(EXIT_FAILURE, "Failed to set terminal deinitializer on exit.");

	/* Set raw mode parameters to termios */
	raw_termios = term.orig_termios;
	term_set_raw_mode_params(&raw_termios);

	/* Enable raw mode with new parameters */
	if (tcsetattr(term.ifd, TCSANOW, &raw_termios) == -1)
		err(EXIT_FAILURE, "Failed to enable raw mode");
}

static void
term_set_raw_mode_params(struct termios *const params)
{
	params->c_iflag &= \
		~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	params->c_oflag &= ~OPOST;
	params->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	params->c_cflag &= ~(CSIZE | PARENB);
	params->c_cflag |= CS8;
	params->c_cc[VMIN] = 1;
}

size_t
term_wait_key(char *const seq, const size_t len)
{
	ssize_t readed;
	/*
	Read input up to specified length.

	We ignore the system call interruption that can occur when the window size is
	changed, for example, in xterm.
	*/
	if ((readed = read(term.ifd, seq, len)) == -1 && errno != EINTR)
		err(EXIT_FAILURE, "Failed to read key sequence");
	/* It is ok to return signed number because of errors check before */
	return readed;
}

void
term_write(const char *const buf, const size_t len)
{
	/* Write buffer with accepted length */
	if (write(term.ofd, buf, len) == -1)
		err(EXIT_FAILURE, "Failed to write %zu bytes to terminal", len);
}
