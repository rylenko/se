# Simple Editor

**Important goals**:

- Ease of configuring, building and use.
- Fast movement and editing.
- The middle ground between readable code and a small code base of up to 3000 lines.
- No dependencies.
- Extensibility.

**Examples of what may be in separate patches**:

- Line numbers on the left.
- Automatic saving.
- Syntax highlighting.
- UTF-8 support.
- Key macros.
- Regular expressions.
- Configuring using `~/.config/se/se.conf` or something like that.

**Must be avoided**:

- Multiple windows support.
- File browser.
- Support for plugins written in another language.
- LSP.

# Usage

Open a file:

```
$ se <path>
```

Normal mode keys:

- `a` - start of line.
- `d` - end of line.
- `e` - go to begin of next word.
- `h` or `Left arrow` - go left.
- `i` - switch to inserting mode.
- `j` or `Down arrow` - go down.
- `k` or `Up arrow` - go up.
- `l` or `Right arrow` - go right.
- `n` - create a line below the current line and move to it.
- `q` - go to begin of previous word.
- (X) `r` - redo last undo;
- `s` - go to end of file.
- (X) `u` - undo last change.
- `w` - go to begin of file.
- `/` - switch to searching mode.
- `Ctrl+d` - delete current line.
- `Ctrl+n` - create a line above the current line and move to it.
- `Ctrl+s` - save.
- `Ctrl+q` - quit. If you changed the file, you will need to either save it or press this key several times.
- `Ctrl+x` - save to spare directory. Useful if no privilege to write to opened file.

You can also repeat a key by pressing `<number><key>`. For example, `5n` will create 5 lines below the cursor.

Inserting mode keys:

- `Esc` - switch to normal mode.
- `Backspace` - delete character before cursor.
- `Enter` - break line.
- Otherwise, if character is printable, the character is inserted.

Searching mode keys:

- `Esc` - switch to normal mode.
- `Backspace` - delete last character in search query.
- `Enter` - search forward.
- `Tab` - search backward.
- Otherwise, if character is printable, the character is inserted to search query.

# Build and install

Build binary:

```
$ make
```

Install:

```
$ make install
```

Build with debug:

```
$ DEBUG=1 make
```

Build and use valgrind:

```
$ make valgrind
```

Clean all build files:

```
$ make clean
```

Regenerate README.md:

```
$ make gen-readme
```

# Configuration

You can set up convenient key bindings and convenient colors in `src/cfg.h`.

