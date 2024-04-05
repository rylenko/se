NAME = se
PREFIX = /usr/local

# XOPEN_SOURCE=500 needed for portable signal handling
CFLAGS = -D_XOPEN_SOURCE=500 -O2 -pedantic -std=c99 -Wall -Werror -Wextra
CC = cc
