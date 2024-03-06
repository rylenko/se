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

- `e` - end of line.
- `h` - go left.
- `i` - switch to insert mode.
- `j` - go down.
- `k` - go up.
- `l` - go right.
- `q` - start of line.
- `s` - end of file.
- `w` - start of file.
- `/` - start or end search.
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
		<th>7</th>
		<th>643</th>
		<th>91</th>
		<th>95</th>
		<th>457</th>
		<th>55</th>
		<th>12796</th>
	</tr><tr>
		<th>C Header</th>
		<th>8</th>
		<th>163</th>
		<th>41</th>
		<th>44</th>
		<th>78</th>
		<th>0</th>
		<th>3332</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>111</th>
		<th>38</th>
		<th>0</th>
		<th>73</th>
		<th>0</th>
		<th>2456</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>80</th>
		<th>16</th>
		<th>14</th>
		<th>50</th>
		<th>0</th>
		<th>1769</th>
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
		<th>22</th>
		<th>1065</th>
		<th>195</th>
		<th>159</th>
		<th>711</th>
		<th>55</th>
    	<th>21594</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/main.c**|**1**|**comment includes**|
|**src/editor.c**|**71**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**82**|**check cursor position fits on the screen**|
|**src/editor.c**|**101**|**should we need to copy it?**|
|**src/row.c**|**124**|**Linked list is better for creating new rows**|
|**src/row.c**|**143**|**create shrink_to_fit for rows_remove**|
