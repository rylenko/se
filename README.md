# Edelweiss

A simple text editor.

# Goals

**Important goals**:

- Ease of configuring, building and use.
- Fast movement and editing.
- Small code base of up to 2000 lines.
- No dependencies.
- Extensibility.

**Examples of what must be in separate patches**:

- Syntax highlighting.
- UTF-8 support.
- Key macros.
- Regular expressions.

**Must be avoided**:

- Multiple windows support.
- File browser.
- Support for plugins written in another language.
- Configuring not via header file.

# Usage

Open a file:

```
$ ew <path>
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
- (X) `CTRL+x` - save to current user's home directory. Useful if no privilege to write to opened file.

You can also repeat a key by pressing `<number><key>`. For example, `5n` will create 5 rows below the cursor.

Insert mode keys:

- `ESC` - switch to normal mode.

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
		<th>12</th>
		<th>1416</th>
		<th>150</th>
		<th>185</th>
		<th>1081</th>
		<th>175</th>
		<th>28445</th>
	</tr><tr>
		<th>C Header</th>
		<th>14</th>
		<th>334</th>
		<th>72</th>
		<th>77</th>
		<th>185</th>
		<th>1</th>
		<th>7327</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>209</th>
		<th>62</th>
		<th>0</th>
		<th>147</th>
		<th>0</th>
		<th>3512</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>83</th>
		<th>15</th>
		<th>13</th>
		<th>55</th>
		<th>0</th>
		<th>1961</th>
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
		<th>2110</th>
		<th>308</th>
		<th>281</th>
		<th>1521</th>
		<th>176</th>
    	<th>42484</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/ed.c**|**1**|**Add local clipboard. Use it in functions.**|
|**src/ed.c**|**2**|**Use linked list for rows array and row's content parts**|
|**src/ed.c**|**3**|**Integrate repetition of keys into handlers**|
|**src/ed.c**|**4**|**Undo operations. Also rename "del" to "remove" where needed**|
|**src/ed.c**|**5**|**Maybe split ed.c into ed_init.c, ed_stat.c, ed_key.c, etc.?**|
|**src/ed.c**|**6**|**Support huge files.**|
|**src/ed.c**|**7**|**Xclip patch to use with local clipboard**|
|**src/ed.c**|**676**|**rows_write**|
|**src/row.h**|**19**|**rename to `rows_remove` if undo is done.**|
|**src/cfg.h**|**44**|**Fix it for another values**|
