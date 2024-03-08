# Vega

A simple text editor.

The name comes from the [star Vega](https://en.wikipedia.org/wiki/Vega).

# Usage

Open a file:

```
$ vega <path>
```

If the file has not yet been created:

```
$ touch <path>
$ vega <path>
```

Normal mode keys:

- `a` - start of line.
- `d` - end of line.
- `e` - go to begin of next word.
- `h` - go left.
- `i` - switch to insert mode.
- `j` - go down.
- `k` - go up.
- `l` - go right.
- `n` - create a line below the current line and move to it.
- `q` - go to begin of previous word.
- `s` - go to end of file.
- `w` - go to begin of file.
- `/` - start or end search.
- `CTRL+n` - create a line above the current line and move to it.
- `CTRL+s` - save.
- `CTRL+q` - quit. If you changed the file, you will need to either save it or press this key several times.
- `<number>g` - Go to line by its number `<number>`.

Insert mode keys:

- `ESC` - switch to normal mode.

# Why

For a simple text editor without dependencies that can edit, save, search in a file and nothing more.

There is no support for multiple windows. Use [tmux](https://github.com/tmux/tmux).

There is no support for plugins written in [Lua](https://en.wikipedia.org/wiki/Lua_(programming_language)) or [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) or anything like that.

Support for syntax highlighting is optional and can be enabled by applying patches.

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
		<th>9</th>
		<th>891</th>
		<th>112</th>
		<th>118</th>
		<th>661</th>
		<th>81</th>
		<th>16942</th>
	</tr><tr>
		<th>C Header</th>
		<th>11</th>
		<th>213</th>
		<th>51</th>
		<th>48</th>
		<th>114</th>
		<th>0</th>
		<th>4245</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>119</th>
		<th>38</th>
		<th>0</th>
		<th>81</th>
		<th>0</th>
		<th>2878</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>80</th>
		<th>15</th>
		<th>13</th>
		<th>52</th>
		<th>0</th>
		<th>1788</th>
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
		<th>13</th>
	</tr></tbody>
	<tfoot><tr>
		<th>Total</th>
		<th>27</th>
		<th>1371</th>
		<th>225</th>
		<th>185</th>
		<th>961</th>
		<th>81</th>
    	<th>27094</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/row.c**|**132**|**Linked list is better for creating new rows**|
|**src/row.c**|**152**|**create shrink_to_fit for rows_remove**|
|**src/cfg.h**|**6**|**Fix it for another values**|
