# Simple Editor

**Important goals**:

- Ease of configuring, building and use.
- Fast movement and editing.
- Small code base of up to 2000 lines.
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
- `i` - switch to insert mode.
- `j` or `Down arrow` - go down.
- `k` or `Up arrow` - go up.
- `l` or `Right arrow` - go right.
- `n` - create a line below the current line and move to it.
- `q` - go to begin of previous word.
- (X) `r` - redo last undo;
- `s` - go to end of file.
- (X) `u` - undo last change.
- `w` - go to begin of file.
- (X) `/` - start or end search.
- `CTRL+d` - delete current line.
- `CTRL+n` - create a line above the current line and move to it.
- `CTRL+s` - save.
- `CTRL+q` - quit. If you changed the file, you will need to either save it or press this key several times.
- `CTRL+x` - save to spare directory. Useful if no privilege to write to opened file.

You can also repeat a key by pressing `<number><key>`. For example, `5n` will create 5 lines below the cursor.

Insert mode keys:

- `ESC` - switch to normal mode.
- `Backspace` - delete character before cursor.
- `Enter` - break line.
- Otherwise, if character is printable, the character is inserted.

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
		<th>19</th>
		<th>1724</th>
		<th>236</th>
		<th>247</th>
		<th>1241</th>
		<th>239</th>
		<th>39219</th>
	</tr><tr>
		<th>C Header</th>
		<th>21</th>
		<th>661</th>
		<th>153</th>
		<th>230</th>
		<th>278</th>
		<th>0</th>
		<th>16339</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>213</th>
		<th>58</th>
		<th>0</th>
		<th>155</th>
		<th>0</th>
		<th>4008</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>69</th>
		<th>14</th>
		<th>12</th>
		<th>43</th>
		<th>0</th>
		<th>1498</th>
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
		<th>47</th>
		<th>2735</th>
		<th>470</th>
		<th>495</th>
		<th>1770</th>
		<th>239</th>
    	<th>62303</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/lines.h**|**14**|**do not forget to rerender after absorb**|
|**src/main.c**|**1**|**[1] Use ptrdiff_t where needed.**|
|**src/main.c**|**2**|**[1] Remove unused includes.**|
|**src/main.c**|**3**|**[1] Try to make code easier and less bloated.**|
|**src/main.c**|**4**|**[1] Add more clear docs and comments.**|
|**src/main.c**|**5**|**[1] Handle non-printable characters.**|
|**src/main.c**|**6**|**[2] Add local clipboard. Use it in functions.**|
|**src/main.c**|**7**|**[2] Xclip patch to use with local clipboard.**|
|**src/main.c**|**8**|**[2] Use linked list for lines array and line's content parts.**|
|**src/main.c**|**9**|**[3] Support huge files.**|
|**src/main.c**|**10**|**[3] Undo operations. Also rename "del" to "remove" where needed.**|
|**src/main.c**|**11**|**[4] API with status codes instead of err.h and tests**|
