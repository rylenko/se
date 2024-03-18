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
		<th>11</th>
		<th>1569</th>
		<th>166</th>
		<th>210</th>
		<th>1193</th>
		<th>215</th>
		<th>32172</th>
	</tr><tr>
		<th>C Header</th>
		<th>13</th>
		<th>344</th>
		<th>74</th>
		<th>79</th>
		<th>191</th>
		<th>1</th>
		<th>7734</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>207</th>
		<th>58</th>
		<th>0</th>
		<th>149</th>
		<th>0</th>
		<th>3668</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>82</th>
		<th>15</th>
		<th>13</th>
		<th>54</th>
		<th>0</th>
		<th>1871</th>
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
		<th>31</th>
		<th>2270</th>
		<th>322</th>
		<th>308</th>
		<th>1640</th>
		<th>216</th>
    	<th>46684</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/ed.c**|**1**|**break row via Enter**|
|**src/ed.c**|**2**|**Add local clipboard. Use it in functions.**|
|**src/ed.c**|**3**|**Use linked list for rows array and row's content parts**|
|**src/ed.c**|**4**|**Integrate repetition of keys into handlers**|
|**src/ed.c**|**5**|**Undo operations. Also rename "del" to "remove" where needed**|
|**src/ed.c**|**6**|**Maybe split ed.c into ed_init.c, ed_stat.c, ed_key.c, etc.?**|
|**src/ed.c**|**7**|**Support huge files.**|
|**src/ed.c**|**8**|**Xclip patch to use with local clipboard**|
|**src/ed.c**|**184**|**union current and previous rows if current is not first**|
|**src/row.h**|**32**|**rename to `rows_remove` if undo is done.**|
|**src/cfg.h**|**50**|**Fix it for another values**|
