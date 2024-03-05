# Vega

A simple text editor without dependencies.

The name comes from the [star Vega](https://en.wikipedia.org/wiki/Vega).

# Syntax highlighting

Not implemented yet. Perhaps this feature will be optional and presented as a patch that can be applied.

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
		<th>394</th>
		<th>58</th>
		<th>54</th>
		<th>282</th>
		<th>24</th>
		<th>8063</th>
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
		<th>Markdown</th>
		<th>2</th>
		<th>21</th>
		<th>10</th>
		<th>0</th>
		<th>11</th>
		<th>0</th>
		<th>526</th>
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
		<th>666</th>
		<th>122</th>
		<th>108</th>
		<th>436</th>
		<th>24</th>
    	<th>13708</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/buf.c**|**32**|**choose a more efficient memory allocation strategy**|
|**src/editor.c**|**48**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**59**|**check cursor position fits on the screen**|
