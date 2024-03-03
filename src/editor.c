#include <assert.h>
#include "buf.h"
#include "editor.h"
#include "term.h"

Editor
editor_new(const char *path)
{
	assert(path);
	return (Editor){ .path = path };
}

void
editor_refresh_scr(const Editor *editor)
{
	Buf buf = buf_alloc();

	/* Prepare for file content */
	term_hide_cur(&buf);
	term_go_home(&buf);

	buf_write(&buf, "Hello,", 6);
	buf_write(&buf, " world!\r\n", 9);
	buf_writef(&buf, "Filename: %s\r\n", editor->path);

	/* Recovery after file content */
	term_show_cur(&buf);

	/* Flush and free the buffer */
	term_flush(&buf);
	buf_free(buf);
}
