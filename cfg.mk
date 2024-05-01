CC = c99

# Default flags.
#
# XOPEN_SOURCE=500 needed to use `sigaction`
CFLAGS = -D_XOPEN_SOURCE=500 -g -pedantic -Wall -Werror -Wextra

# OpenBSD flags. Uncomment to use
# CFLAGS = -O2 -pedantic -Wall -Werror -Wextra

NAME = se
PREFIX = /usr/local
