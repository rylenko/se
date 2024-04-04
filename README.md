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
		<th>2402</th>
		<th>337</th>
		<th>440</th>
		<th>1625</th>
		<th>260</th>
		<th>57050</th>
	</tr><tr>
		<th>C Header</th>
		<th>14</th>
		<th>456</th>
		<th>120</th>
		<th>145</th>
		<th>191</th>
		<th>0</th>
		<th>11277</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>237</th>
		<th>66</th>
		<th>0</th>
		<th>171</th>
		<th>0</th>
		<th>4802</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>68</th>
		<th>14</th>
		<th>12</th>
		<th>42</th>
		<th>0</th>
		<th>1405</th>
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
		<th>3231</th>
		<th>546</th>
		<th>603</th>
		<th>2082</th>
		<th>260</th>
    	<th>75773</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/main.c**|**1**|**[1] Optimize and review searching code.**|
|**src/main.c**|**2**|**[1] Normalize searching with tabs.**|
|**src/main.c**|**3**|**[1] Test where we do not need win_scroll.**|
|**src/main.c**|**4**|**[1] Fix treating tabs behind offset as one character: erased as 1 character, other lines move slowly when inserting multiple tabs.**|
|**src/main.c**|**5**|**[1] Try to make code easier and less bloated.**|
|**src/main.c**|**6**|**[1] Add more clear docs and comments.**|
|**src/main.c**|**7**|**[1] Handle non-printable characters.**|
|**src/main.c**|**8**|**[2] Add local clipboard. Use it in functions.**|
|**src/main.c**|**9**|**[2] Xclip patch to use with local clipboard.**|
|**src/main.c**|**10**|**[2] Use linked list for lines array and line's content parts.**|
|**src/main.c**|**11**|**[3] Support huge files.**|
|**src/main.c**|**12**|**[3] Undo operations. Also rename "del" to "remove" where needed.**|
|**src/main.c**|**13**|**[4] API with status codes instead of err.h and tests**|
