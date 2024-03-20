# Simple Editor

**Important goals**:

- Ease of configuring, building and use.
- Fast movement and editing.
- Small code base of up to 2000 lines.
- No dependencies.
- Extensibility.

**Examples of what may be in separate patches**:

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

- `a` - start of row.
- `d` - end of row.
- `e` - go to begin of next token.
- `h` - go left.
- `i` - switch to insert mode.
- `j` - go down.
- `k` - go up.
- `l` - go right.
- `n` - create a row below the current row and move to it.
- `q` - go to begin of previous token.
- (X) `r` - redo last undo;
- `s` - go to end of file.
- (X) `u` - undo last change.
- `w` - go to begin of file.
- (X) `/` - start or end search.
- `CTRL+d` - delete current row.
- `CTRL+n` - create a row above the current row and move to it.
- `CTRL+s` - save.
- `CTRL+q` - quit. If you changed the file, you will need to either save it or press this key several times.
- `CTRL+x` - save to spare directory. Useful if no privilege to write to opened file.

You can also repeat a key by pressing `<number><key>`. For example, `5n` will create 5 rows below the cursor.

Insert mode keys:

- `ESC` - switch to normal mode.
- `Backspace` - erase character before cursor.
- `Enter` - break row.
- Otherwise the character is inserted.

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
		<th>21</th>
		<th>1551</th>
		<th>136</th>
		<th>166</th>
		<th>1249</th>
		<th>206</th>
		<th>33155</th>
	</tr><tr>
		<th>C Header</th>
		<th>23</th>
		<th>544</th>
		<th>130</th>
		<th>125</th>
		<th>289</th>
		<th>1</th>
		<th>11403</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>209</th>
		<th>58</th>
		<th>0</th>
		<th>151</th>
		<th>0</th>
		<th>3714</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>70</th>
		<th>14</th>
		<th>12</th>
		<th>44</th>
		<th>0</th>
		<th>1534</th>
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
		<th>51</th>
		<th>2442</th>
		<th>347</th>
		<th>309</th>
		<th>1786</th>
		<th>207</th>
    	<th>51045</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/main.c**|**1**|**Integrate repetition of keys into handlers**|
|**src/main.c**|**2**|**Add local clipboard. Use it in functions.**|
|**src/main.c**|**3**|**Use linked list for rows array and row's content parts. Benchmark!**|
|**src/main.c**|**4**|**Undo operations. Also rename "del" to "remove" where needed**|
|**src/main.c**|**5**|**Xclip patch to use with local clipboard**|
|**src/main.c**|**6**|**Support huge files.**|
|**src/cfg.h**|**51**|**Fix it for another values**|
|**src/rows.h**|**22**|**rename to `rows_remove` if undo is done.**|
|**src/rows.c**|**23**|**Copy a smaller portion of a row to the row abome or below**|
