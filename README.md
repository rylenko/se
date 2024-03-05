# Vega

A simple text editor without dependencies. Supports syntax highlighting.

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
		<th>6</th>
		<th>393</th>
		<th>58</th>
		<th>54</th>
		<th>281</th>
		<th>24</th>
		<th>8039</th>
	</tr><tr>
		<th>C Header</th>
		<th>5</th>
		<th>117</th>
		<th>31</th>
		<th>38</th>
		<th>48</th>
		<th>0</th>
		<th>2552</th>
	</tr><tr>
		<th>Makefile</th>
		<th>2</th>
		<th>57</th>
		<th>12</th>
		<th>9</th>
		<th>36</th>
		<th>0</th>
		<th>1165</th>
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
		<th>328</th>
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
		<th>18</th>
		<th>648</th>
		<th>116</th>
		<th>107</th>
		<th>425</th>
		<th>24</th>
    	<th>13325</th>
	</tr></tfoot>
	</table>

# Todo

|Path|Line|Description|
|-|-|-|
|**src/buf.c**|**32**|**choose a more efficient memory allocation strategy**|
|**src/editor.c**|**47**|**Avoid many reallocations in `buf_write`**|
|**src/editor.c**|**58**|**check cursor position fits on the screen**|
