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

# Todo

|Path|Line|Description|
|-|-|-|
|**src/main.c**|**1**|**v0.3: Check using valgrind and eyes that all memory is freed on errors.**|
|**src/main.c**|**2**|**v0.3: Split functions into smaller functions, which "do one thing and do it well".**|
|**src/main.c**|**3**|**v0.3: Don't say in comments what can be clearly stated in code.**|
|**src/main.c**|**4**|**v0.4: Create Cell struct to handle all symbols including UTF-8. Create structs Win->Renders->Render->Cells->Cell. Rerender lines on window side**|
|**src/main.c**|**5**|**v0.4: Use linked list for lines array and line's content parts.**|
|**src/main.c**|**6**|**v0.4: Remember last position per line.**|
|**src/main.c**|**7**|**v0.4: Rewrite moving functions with more unambiguous behavior and minimal scrolling.**|
|**src/main.c**|**8**|**v0.5: Undo operations. Also rename "del" to "remove" where needed.**|
|**src/main.c**|**9**|**v0.5: Add key settings for escape sequences. For example, CFG_KEY_MV_UP_2 = "..."**|
|**src/main.c**|**10**|**v0.5: Add local clipboard. Use it in functions.**|
|**src/main.c**|**11**|**v0.5: Xclip patch to use with local clipboard.**|
|**src/main.c**|**12**|**v0.6: Support huge files: read chunks or try mmap**|
|**src/main.c**|**13**|**v0.6: Add tests.**|
|**src/main.c**|**14**|**v0.7: Make code patching easier.**|
|**src/main.c**|**15**|**v0.7: Add more error codes in docs.**|
|**src/main.c**|**16**|**v0.7: Save to spare dir on error.**|
