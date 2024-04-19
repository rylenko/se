.POSIX:

include cfg.mk

# Code files
SRC = src/ed.c src/esc.c src/file.c src/main.c src/mode.c src/path.c \
	src/str.c src/term.c src/vec.c src/win.c src/word.c
OBJ = $(SRC:.c=.o)

# Paths
GEN_README_PATH = ./readme-gen/run
VALGRIND_LOG_PATH = /tmp/se-valgrind.log

# Build executable
all: $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

# Build object file from source file.
#
# Target equals to GNU `src/%.o: src/%.c`
.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

# Clean all after build
clean:
	rm -f $(NAME) $(OBJ)

gen-readme:
	$(GEN_README_PATH)

install: $(NAME)
	mkdir -p $(PREFIX)/bin
	cp $(NAME) $(PREFIX)/bin/$(NAME)
	chmod 755 $(PREFIX)/bin/$(NAME)

uninstall:
	rm $(PREFIX)/bin/$(NAME)

# Check errors and memory leaks using valgrind
valgrind: $(NAME)
	# Use valgrind to check memory leaks
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=$(VALGRIND_LOG_PATH) \
		./$(NAME) ./README.md

	# Show valgrind output and remove temp and out files
	less $(VALGRIND_LOG_PATH)
	rm -f $(VALGRIND_LOG_PATH)

.PHONY: all clean gen-readme install uninstall valgrind
