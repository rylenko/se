.POSIX:

include config.mk

# Code files
SRC = src/buf.c src/color.c src/editor.c src/err.c src/main.c src/row.c src/term.c
OBJ = $(SRC:.c=.o)

# Paths
ROOT_DIR = $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
GEN_README_PATH = $(ROOT_DIR)readme-gen/run

# Valgrind
VALGRIND_OUT = "valgrind-out"
VALGRIND_TEMP = "valgrind-temp"

# Build executable
all: $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

# Build object file from source file
src/%.o: src/%.c
ifdef DEBUG
	$(CC) -c -g $(CFLAGS) -o $@ $<
else
	$(CC) -c $(CFLAGS) -o $@ $<
endif

# Object file dependencies
src/buf.o: src/buf.h src/err.h
src/color.o: src/buf.h src/color.h
src/editor.o: src/color.h src/editor.h src/key.h src/math.h src/row.h src/term.h
src/err.o: src/err.h
src/main.o: src/buf.h src/editor.h src/err.h src/term.h
src/row.o: src/err.h src/row.h
src/term.o: src/err.h src/term.h

# Clean all after build
clean:
	rm -f $(NAME) $(OBJ)

# Generate README.md file
gen-readme:
	shellcheck $(GEN_README_PATH)
	$(GEN_README_PATH)

# Install after build
install: all
	mkdir -p $(PREFIX)/bin
	cp $(NAME) $(PREFIX)/bin/$(NAME)
	chmod 755 $(PREFIX)/bin/$(NAME)

# Uninstall
uninstall:
	rm $(PREFIX)/bin/$(NAME)

# Valgrind
valgrind: all
	# Create temp file
	touch $(VALGRIND_TEMP)
	echo "Hello, world\nNext line\n" > $(VALGRIND_TEMP)

	# Use valgrind to check memory leaks
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=$(VALGRIND_OUT) \
		./$(NAME) $(VALGRIND_TEMP)

	# Show valgrind output and remove temp and out files
	less $(VALGRIND_OUT)
	rm -f $(VALGRIND_TEMP) $(VALGRIND_OUT)

.PHONY: all clean gen-readme install valgrind uninstall
