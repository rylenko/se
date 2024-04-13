CC = cc

# Default flags.
#
# XOPEN_SOURCE=500 needed to use `sigaction`
CFLAGS = -D_XOPEN_SOURCE=500 -O2 -pedantic -std=c99 -Wall -Werror -Wextra
# OpenBSD flags. Uncomment to use
# CFLAGS = -O2 -pedantic -std=c99 -Wall -Werror -Wextra

NAME = se
PREFIX = /usr/local
