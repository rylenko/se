# Vega

A simple text editor.

The name comes from the [star Vega](https://en.wikipedia.org/wiki/Vega).

# Why

For a simple text editor without dependencies that can edit, save, search in a file and nothing more.

There is no support for multiple windows. Use [tmux](https://github.com/tmux/tmux).

There is no support for plugins written in [Lua](https://en.wikipedia.org/wiki/Lua_(programming_language)) or [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) or anything like that.

Support for syntax highlighting is optional and can be enabled by applying patches.

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

The control keys are very simple:

- `CTRL+s` for save.
- `CTRL+q` for exit. If you changed the file, you will need to either save it or press this key several times.
- `CTRL+/` for search.
- `ESC` to stop searching.

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
		<th>6</th>
		<th>407</th>
		<th>62</th>
		<th>59</th>
		<th>286</th>
		<th>25</th>
		<th>8295</th>
	</tr><tr>
		<th>C Header</th>
		<th>6</th>
		<th>126</th>
		<th>33</th>
		<th>39</th>
		<th>54</th>
		<th>0</th>
		<th>2703</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>85</th>
		<th>34</th>
		<th>0</th>
		<th>51</th>
		<th>0</th>
		<th>1954</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>57</th>
		<th>12</th>
		<th>9</th>
		<th>36</th>
		<th>0</th>
		<th>1175</th>
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
		<th>19</th>
		<th>743</th>
		<th>150</th>
		<th>113</th>
		<th>480</th>
		<th>25</th>
    	<th>15368</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/buf.c**|**32**|**choose a more efficient memory allocation strategy**|
|**src/editor.c**|**49**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**60**|**check cursor position fits on the screen**|
|**src/editor.c**|**87**|**do not read all file. Instead read chunks**|
