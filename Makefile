.POSIX:

include config.mk

# Code files
SRC = src/err.c src/main.c src/term.c
OBJ = $(SRC:.c=.o)

# Paths
ROOT_DIR = $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
GEN_README_PATH = $(ROOT_DIR)readme-gen/run

# Build executable
all: $(OBJ) gen-readme
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

# Build object file from source file
src/%.o: src/%.c
ifdef DEBUG
	$(CC) -c -g $(CFLAGS) -o $@ $<
else
	$(CC) -c $(CFLAGS) -o $@ $<
endif

# Object file dependencies
src/main.o: src/err.h src/term.h
src/term.o: src/err.h src/term.h
src/err.o: src/err.h

# Clean all after build
clean:
	rm -f $(NAME) $(OBJ)

# Generate README.md file
gen-readme:
	shellcheck $(GEN_README_PATH)
	$(GEN_README_PATH)

# Install after build
install: $(NAME)
	mkdir -p $(PREFIX)/bin
	cp -f $< $(PREFIX)/bin/$<
	chmod 755 $(PREFIX)/bin/$<

# Uninstall
uninstall:
	rm -f $(PREFIX)/bin/$(NAME)

.PHONY: all clean install uninstall
