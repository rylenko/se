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
		<th>516</th>
		<th>75</th>
		<th>75</th>
		<th>366</th>
		<th>40</th>
		<th>10380</th>
	</tr><tr>
		<th>C Header</th>
		<th>7</th>
		<th>140</th>
		<th>35</th>
		<th>44</th>
		<th>61</th>
		<th>0</th>
		<th>2969</th>
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
		<th>79</th>
		<th>16</th>
		<th>14</th>
		<th>49</th>
		<th>0</th>
		<th>1698</th>
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
		<th>21</th>
		<th>914</th>
		<th>173</th>
		<th>139</th>
		<th>602</th>
		<th>40</th>
    	<th>18744</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/buf.c**|**32**|**choose a more efficient memory allocation strategy**|
|**src/main.c**|**1**|**comment includes**|
|**src/editor.c**|**65**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**76**|**check cursor position fits on the screen**|
|**src/editor.c**|**97**|**should we need to copy it?**|
