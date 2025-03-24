# Simple Editor

**Important goals**:

- Ease of configuring, building and use.
- Fast movement and editing.
- The middle ground between readable code and a small code base.
- No dependencies.
- Extensibility.

**Examples of what may be in separate patches**:

- Line numbers on the left.
- Automatic saving.
- Syntax highlighting.
- Key macros.
- Regular expressions.
- Configuring using `~/.config/se/se.conf` or something like that.

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
- `j`, `Down arrow` or by moving the mouse wheel down - go down.
- `k` or `Up arrow` or by moving the mouse wheel up - go up.
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
- `Enter` - Search forward if a query was previously entered in the search mode.
- `Tab` - Search backward if a query was previously entered in the search mode.

You can also repeat a key by pressing `<number><key>`. For example, `5n` will create 5 lines below the cursor.

Inserting mode keys:

- `Esc` - switch to normal mode.
- `Backspace` - delete character before cursor.
- `Enter` - break line.
- Otherwise, if character is printable, the character is inserted.

Searching mode keys:

- `Esc` - Cancel searching and switch to normal mode.
- `Backspace` - delete last character in search query.
- `Enter` - End query input, switch to normal mode and search forward once.
- Otherwise, if character is printable, the character is inserted to search query.

# Configuration

You can set up convenient key bindings and convenient colors in `src/cfg.h`. Note that after changes you need to build and install again.

Popular changes (I will make separate patches if there are many differences with the default config):
- In XTerm, **backspace** is encoded as 8. Therefore, you need to replace `CFG_KEY_DEL_CHAR` with 8.

# Build and install

You can set up convenient building flags in `cfg.mk`. For example, if you want to debug, you need to add `-g` to flags and remove optimizations. Or, if you want to build editor for OpenBSD, you need to uncomment some lines in `cfg.mk`.

Build binary:

```
$ make
```

Install. Most likely you will need `sudo`, `doas` or something like that before the command:

```
$ make install
```

Build and use valgrind:

```
$ make valgrind
```

Clean all build files:

```
$ make clean
```

Regenerate README.md. Requires awk to collect todos:

```
$ make gen-readme
```

# Codying style

Trying to follow a combination of [OpenBSD](https://man.openbsd.org/style) and [Linux](https://www.kernel.org/doc/html/v4.10/process/coding-style.html) styles with priority given to the [OpenBSD](https://man.openbsd.org/style) style.

# Todo

|Path|Line|Description|
|-|-|-|
|**src/main.c**|**1**|**perror errors in goto-cleanups**|
|**src/main.c**|**2**|**Create Cell struct to handle all symbols including UTF-8. Create structs Win->Renders->Render->Cells->Cell. Rerender lines on window side**|
|**src/main.c**|**3**|**Use linked list for lines array and line's content parts.**|
|**src/main.c**|**4**|**Remember last position per line.**|
|**src/main.c**|**5**|**Open binary files and files with ^M at the end of line.**|
|**src/main.c**|**6**|**Undo operations. Also rename "del" to "remove" where needed.**|
|**src/main.c**|**7**|**Add key settings for escape sequences. For example, CFG_KEY_MV_UP_2 = "..."**|
|**src/main.c**|**8**|**Add local clipboard. Use it in functions.**|
|**src/main.c**|**9**|**Xclip patch to use with local clipboard.**|
|**src/main.c**|**10**|**Support huge files: read chunks or try mmap**|
|**src/main.c**|**11**|**Add tests.**|
|**src/main.c**|**12**|**Make code patching easier.**|
|**src/main.c**|**13**|**Add more error codes in docs.**|
|**src/main.c**|**14**|**Save to spare dir on error.**|
