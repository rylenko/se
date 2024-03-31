#include "line.h"
#include "lines.h"
#include "math.h"
#include "win.h"
#include "win_del.h"
#include "win_mv.h"

int
win_del_line(Win *const win, size_t times)
{
	if (1 >= win->file.lines.cnt) {
		return -1;
	} else if (times > 0) {
		/* Get real repeat times */
		times = MIN(times, win->file.lines.cnt - win->offset.rows - win->cur.row);

		/* The file must contain at least one line */
		if (times == win->file.lines.cnt)
			times--;

		/* Remove column offsets */
		win_mv_to_begin_of_line(win);

		/* Times is never zero here */
		while (times-- > 0)
			/* Delete lines */
			file_del(&win->file, win->offset.rows + win->cur.row);

		/* Move up if we deleted the last line and stayed there */
		if (win->offset.rows + win->cur.row == win->file.lines.cnt)
			win_mv_up(win, 1);
	}
	return 0;
}
