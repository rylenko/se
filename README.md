# Vega

A simple text editor without dependencies.

The name comes from the [star Vega](https://en.wikipedia.org/wiki/Vega).

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
		<th>5</th>
		<th>353</th>
		<th>54</th>
		<th>52</th>
		<th>247</th>
		<th>22</th>
		<th>7183</th>
	</tr><tr>
		<th>C Header</th>
		<th>4</th>
		<th>94</th>
		<th>26</th>
		<th>30</th>
		<th>38</th>
		<th>0</th>
		<th>2122</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>56</th>
		<th>12</th>
		<th>9</th>
		<th>35</th>
		<th>0</th>
		<th>1106</th>
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
		<th>Markdown</th>
		<th>2</th>
		<th>13</th>
		<th>6</th>
		<th>0</th>
		<th>7</th>
		<th>0</th>
		<th>268</th>
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
		<th>16</th>
		<th>584</th>
		<th>107</th>
		<th>97</th>
		<th>380</th>
		<th>22</th>
    	<th>11920</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/buf.c**|**32**|**choose a more efficient memory allocation strategy**|
|**src/editor.c**|**42**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**57**|**check cursor position fits on the screen**|
