CC = c99

# Default flags.
#
# XOPEN_SOURCE=500 needed to use `sigaction`
CFLAGS = -D_XOPEN_SOURCE=500 -O2 -pedantic -Wall -Werror -Wextra \
	-Wno-implicit-fallthrough

# OpenBSD flags. Uncomment to use
# CFLAGS = -O2 -pedantic -Wall -Werror -Wextra

NAME = se
PREFIX = /usr/local
