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

int
term_deinit(void)
{
	/* Restore original termios parameters to disable raw mode */
	int ret = tcsetattr(term.ifd, TCSANOW, &term.orig_termios);
	return ret;
}

int
term_get_win_size(struct winsize *const win_size)
{
	/* Get window size using file descriptor. Ignore several success values */
	int ret = ioctl(term.ofd, TIOCGWINSZ, win_size);
	if (-1 == ret)
		return -1;
	return 0;
}

int
term_init(const int ifd, const int ofd)
{
	int ret;
	struct termios raw_termios;

	/* Set file descriptors */
	term.ifd = ifd;
	term.ofd = ofd;

	/* Save the original termios parameters */
	ret = tcgetattr(ifd, &term.orig_termios);
	if (-1 == ret)
		return -1;

	/* Set terminal deinitializer on exit */
	ret = atexit((void (*)(void))term_deinit);
	if (0 != ret)
		return -1;

	/* Set raw mode parameters to termios */
	raw_termios = term.orig_termios;
	term_set_raw_mode_params(&raw_termios);

	/* Enable raw mode with new parameters */
	ret = tcsetattr(term.ifd, TCSANOW, &raw_termios);
	if (-1 == ret)
		return -1;
	return 0;
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

ssize_t
term_wait_key(char *const seq, const size_t len)
{
	/* Read input up to specified length */
	ssize_t readed = read(term.ifd, seq, len);
	/*
	We ignore the system call interruption that can occur when the window size is
	changed, for example, in xterm.
	*/
	if (-1 == readed && errno != EINTR)
		return -1;
	return readed;
}

ssize_t
term_write(const char *const buf, const size_t len)
{
	/* Write buffer with accepted length */
	ssize_t len = write(term.ofd, buf, len);
	return len;
}
