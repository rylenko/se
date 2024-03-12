# Edelweiss

A simple text editor.

# Why

For a simple text editor without dependencies that can edit, save, search in a file and nothing more.

There is no support for multiple windows. Use [tmux](https://github.com/tmux/tmux).

There is no support for plugins written in [Lua](https://en.wikipedia.org/wiki/Lua_(programming_language)) or [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) or anything like that.

Support for syntax highlighting is optional and can be enabled by applying patches.

# Build and install

Build binary:

```
$ make
```

Install:

```
$ make install
```

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
- `n` - create a row  below the current row and move to it.
- `q` - go to begin of previous token.
- `s` - go to end of file.
- `w` - go to begin of file.
- `/` - start or end search.
- `CTRL+n` - create a row above the current row and move to it.
- `CTRL+s` - save.
- `CTRL+q` - quit. If you changed the file, you will need to either save it or press this key several times.
- `<index>g` - Go to row by its index `<index>`.

Insert mode keys:

- `ESC` - switch to normal mode.

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
		<th>1211</th>
		<th>143</th>
		<th>155</th>
		<th>913</th>
		<th>121</th>
		<th>23223</th>
	</tr><tr>
		<th>C Header</th>
		<th>13</th>
		<th>301</th>
		<th>66</th>
		<th>61</th>
		<th>174</th>
		<th>0</th>
		<th>6541</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>129</th>
		<th>42</th>
		<th>0</th>
		<th>87</th>
		<th>0</th>
		<th>2740</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>83</th>
		<th>15</th>
		<th>13</th>
		<th>55</th>
		<th>0</th>
		<th>1900</th>
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
		<th>32</th>
		<th>1792</th>
		<th>275</th>
		<th>235</th>
		<th>1282</th>
		<th>121</th>
    	<th>35643</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/cfg.h**|**36**|**Fix it for another values**|
|**src/row.c**|**130**|**Linked list is better for creating new rows**|
|**src/row.c**|**150**|**create shrink_to_fit for rows_remove**|
|**src/ed.c**|**485**|**Check errors**|
