# Simple Editor

**Important goals**:

- Ease of configuring, building and use.
- Fast movement and editing.
- The middle ground between readable code and a small code base of up to 3000 lines of code.
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
- `Enter` - Search forward if a query was previously entered in the search mode..
- `Tab` - Search backward if a query was previously entered in the search mode..

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

You can set up convenient key bindings and convenient colors in `src/cfg.h`.


# Build and install

Build binary:

```
$ make
```

Install. Most likely you will need `sudo`, `doas` or something like that before the command:

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


# Lines of code

<table id="scc-table">
	<thead><tr>
		<th>Language</th>
		<th>Files</th>
		<th>Lines</th>
		<th>Blank</th>
		<th>Comment</th>
		<th>Code</th>
		<th>Complexity</th>
		<th>Bytes</th>
	</tr></thead>
	<tbody><tr>
		<th>C</th>
		<th>12</th>
		<th>2538</th>
		<th>349</th>
		<th>451</th>
		<th>1738</th>
		<th>284</th>
		<th>60544</th>
	</tr><tr>
		<th>C Header</th>
		<th>14</th>
		<th>486</th>
		<th>126</th>
		<th>152</th>
		<th>208</th>
		<th>0</th>
		<th>12021</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>243</th>
		<th>70</th>
		<th>0</th>
		<th>173</th>
		<th>0</th>
		<th>5400</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>69</th>
		<th>14</th>
		<th>13</th>
		<th>42</th>
		<th>0</th>
		<th>1476</th>
	</tr><tr>
		<th>Shell</th>
		<th>1</th>
		<th>52</th>
		<th>8</th>
		<th>5</th>
		<th>39</th>
		<th>0</th>
		<th>1008</th>
	</tr><tr>
		<th>AWK</th>
		<th>1</th>
		<th>14</th>
		<th>1</th>
		<th>1</th>
		<th>12</th>
		<th>0</th>
		<th>220</th>
	</tr><tr>
		<th>gitignore</th>
		<th>1</th>
		<th>2</th>
		<th>0</th>
		<th>0</th>
		<th>2</th>
		<th>0</th>
		<th>11</th>
	</tr></tbody>
	<tfoot><tr>
		<th>Total</th>
		<th>33</th>
		<th>3404</th>
		<th>568</th>
		<th>622</th>
		<th>2214</th>
		<th>284</th>
    	<th>80680</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/main.c**|**1**|**v0.2: Handle mouse wheel scrolling**|
|**src/main.c**|**2**|**v0.2: Test on other distros/OS and other terminals**|
|**src/main.c**|**3**|**v0.3: Create Cell struct to handle all symbols including UTF-8. Create structs Win->Renders->Render->Cells->Cell. Rerender lines on window side**|
|**src/main.c**|**4**|**v0.3: Use linked list for lines array and line's content parts.**|
|**src/main.c**|**5**|**v0.3: Undo operations. Also rename "del" to "remove" where needed.**|
|**src/main.c**|**6**|**v0.4: Add local clipboard. Use it in functions.**|
|**src/main.c**|**7**|**v0.4: Xclip patch to use with local clipboard.**|
|**src/main.c**|**8**|**v0.5: Add more clear docs and comments.**|
|**src/main.c**|**9**|**v0.5: Support huge files.**|
|**src/main.c**|**10**|**v0.6: API with status codes instead of err.h. Add tests**|
