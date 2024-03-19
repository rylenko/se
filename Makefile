.POSIX:

include cfg.mk

# Code files
SRC = src/buf.c src/cur.c src/ed.c src/ed_del.c src/ed_draw.c src/ed_ins.c \
	src/ed_key.c src/ed_mv.c src/ed_quit.c src/ed_save.c src/ed_sig.c \
	src/ed_term.c src/main.c src/mode.c src/raw_color.c src/raw_key.c src/row.c \
	src/rows.c src/str.c src/term.c src/word.c
OBJ = $(SRC:.c=.o)

# Paths
GEN_README_PATH = $(ROOT_DIR)readme-gen/run
README_PATH = $(ROOT_DIR)README.md
ROOT_DIR = $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# Valgrind
VALGRIND_OUT_PATH = valgrind-out

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
	cp $(NAME) $(PREFIX)/bin/$(NAME)
	chmod 755 $(PREFIX)/bin/$(NAME)

# Uninstall
uninstall:
	rm $(PREFIX)/bin/$(NAME)

# Valgrind
valgrind: $(NAME)
	# Use valgrind to check memory leaks
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=$(VALGRIND_OUT_PATH) \
		./$(NAME) $(README_PATH)

	# Show valgrind output and remove temp and out files
	less $(VALGRIND_OUT_PATH)
	rm -f $(VALGRIND_OUT_PATH)

.PHONY: all clean gen-readme install uninstall valgrind
