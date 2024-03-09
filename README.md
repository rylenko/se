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
- `<number>g` - Go to row by its number `<number>`.

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
		<th>11</th>
		<th>968</th>
		<th>121</th>
		<th>125</th>
		<th>722</th>
		<th>90</th>
		<th>18403</th>
	</tr><tr>
		<th>C Header</th>
		<th>13</th>
		<th>239</th>
		<th>56</th>
		<th>51</th>
		<th>132</th>
		<th>0</th>
		<th>4722</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>119</th>
		<th>38</th>
		<th>0</th>
		<th>81</th>
		<th>0</th>
		<th>2870</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>82</th>
		<th>15</th>
		<th>13</th>
		<th>54</th>
		<th>0</th>
		<th>1864</th>
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
		<th>31</th>
		<th>1476</th>
		<th>239</th>
		<th>195</th>
		<th>1042</th>
		<th>90</th>
    	<th>29100</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/ed.c**|**120**|**check x coordinate after window resizing**|
|**src/ed.c**|**303**|**add empty line if there is not lines readed**|
|**src/row.c**|**132**|**Linked list is better for creating new rows**|
|**src/row.c**|**152**|**create shrink_to_fit for rows_remove**|
|**src/cfg.h**|**6**|**Fix it for another values**|
