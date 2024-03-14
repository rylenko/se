# Edelweiss

A simple text editor.

# Why

For a simple text editor without dependencies that can edit, save, search in a file and nothing more.

There is no support for multiple windows. Use [tmux](https://github.com/tmux/tmux).

There is no support for plugins written in [Lua](https://en.wikipedia.org/wiki/Lua_(programming_language)) or [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) or anything like that.

Support for syntax highlighting is optional and can be enabled by applying patches.

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
- `s` - go to end of file.
- (X) `u` - undo last change.
- `w` - go to begin of file.
- (X) `/` - start or end search.
- `CTRL+d` - delete current row.
- `CTRL+n` - create a row above the current row and move to it.
- `CTRL+s` - save.
- `CTRL+q` - quit. If you changed the file, you will need to either save it or press this key several times.

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

# Syntax highlighting patches

Later.

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
		<th>1266</th>
		<th>144</th>
		<th>164</th>
		<th>958</th>
		<th>136</th>
		<th>24979</th>
	</tr><tr>
		<th>C Header</th>
		<th>14</th>
		<th>316</th>
		<th>69</th>
		<th>64</th>
		<th>183</th>
		<th>1</th>
		<th>6831</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>183</th>
		<th>60</th>
		<th>0</th>
		<th>123</th>
		<th>0</th>
		<th>3374</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>83</th>
		<th>15</th>
		<th>13</th>
		<th>55</th>
		<th>0</th>
		<th>1947</th>
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
		<th>1916</th>
		<th>297</th>
		<th>247</th>
		<th>1372</th>
		<th>137</th>
    	<th>38370</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/row.h**|**34**|**rename to `rows_remove` if undo is done.**|
|**src/cfg.h**|**36**|**Fix it for another values**|
|**src/ed.c**|**1**|**Add local clipboard. Use it in functions.**|
|**src/ed.c**|**2**|**Use linked list for rows array and row's content parts**|
|**src/ed.c**|**3**|**Integrate repetition of keys into handlers**|
|**src/ed.c**|**4**|**Undo operations. Also rename "del" to "remove" where needed**|
|**src/ed.c**|**5**|**Xclip patch to use with local clipboard**|
|**src/ed.c**|**506**|**Check errors**|
