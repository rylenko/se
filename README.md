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

General keys:

- `CTRL+s` - save.
- `CTRL+q` - quit. If you changed the file, you will need to either save it or press this key several times.

Navigation keys:

- `CTRL+g` - go to line by its number.
- `CTRL+/` - start or end search.

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
		<th>6</th>
		<th>424</th>
		<th>64</th>
		<th>62</th>
		<th>298</th>
		<th>26</th>
		<th>8715</th>
	</tr><tr>
		<th>C Header</th>
		<th>6</th>
		<th>127</th>
		<th>33</th>
		<th>39</th>
		<th>55</th>
		<th>0</th>
		<th>2721</th>
	</tr><tr>
		<th>Markdown</th>
		<th>2</th>
		<th>91</th>
		<th>38</th>
		<th>0</th>
		<th>53</th>
		<th>0</th>
		<th>1990</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>78</th>
		<th>16</th>
		<th>14</th>
		<th>48</th>
		<th>0</th>
		<th>1610</th>
	</tr><tr>
		<th>Plain Text</th>
		<th>1</th>
		<th>77</th>
		<th>0</th>
		<th>0</th>
		<th>77</th>
		<th>0</th>
		<th>4453</th>
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
		<th>20</th>
		<th>865</th>
		<th>160</th>
		<th>121</th>
		<th>584</th>
		<th>26</th>
    	<th>20730</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/buf.c**|**32**|**choose a more efficient memory allocation strategy**|
|**src/editor.c**|**55**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**66**|**check cursor position fits on the screen**|
