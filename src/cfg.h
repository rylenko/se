#ifndef _CFG_H
#define _CFG_H

#include "color.h"

/* Helpers for configuration. */
enum {
	CTRL_OFFSET = 96,
};

/* Different editor settings. */
enum {
	CFG_DIRTY_FILE_QUIT_PRESSES_CNT = 4, /* Press to exit without saving */
	CFG_SPARE_PATH_MAX_LEN = 255, /* Max length of formatted spare save path */
	CFG_TAB_SIZE = 8, /* Count of spaces, which equals to one tab */
};

/* Ascii keys to control the editor. */
enum {
	/* Row management */
	CFG_KEY_DEL_CHAR = 127, /* Backspace */
	CFG_KEY_DEL_LINE = 'd' - CTRL_OFFSET, /* CTRL-d */
	CFG_KEY_INS_LINE_BREAK = 13, /* Enter */
	CFG_KEY_INS_LINE_BELOW = 'n',
	CFG_KEY_INS_LINE_ON_TOP = 'n' - CTRL_OFFSET, /* CTRL-n */

	/* Modes switching */
	CFG_KEY_MODE_INS_TO_NORM = 27, /* Escape */
	CFG_KEY_MODE_NORM_TO_INS = 'i',
	CFG_KEY_MODE_NORM_TO_SEARCH = '/',
	CFG_KEY_MODE_SEARCH_TO_NORM = 13, /* Enter */
	CFG_KEY_MODE_SEARCH_TO_NORM_CANCEL = 27, /* Escape */

	/* Movement */
	CFG_KEY_MV_TO_BEGIN_OF_FILE = 'w',
	CFG_KEY_MV_TO_BEGIN_OF_LINE = 'a',
	CFG_KEY_MV_DOWN = 'j',
	CFG_KEY_MV_TO_END_OF_FILE = 's',
	CFG_KEY_MV_TO_END_OF_LINE = 'd',
	CFG_KEY_MV_LEFT = 'h',
	CFG_KEY_MV_TO_NEXT_WORD = 'e',
	CFG_KEY_MV_TO_PREV_WORD = 'q',
	CFG_KEY_MV_RIGHT = 'l',
	CFG_KEY_MV_UP = 'k',

	/* Save or quit */
	CFG_KEY_QUIT = 'q' - CTRL_OFFSET, /* CTRL-q */
	CFG_KEY_SAVE = 's' - CTRL_OFFSET, /* CTRL-s */
	CFG_KEY_SAVE_TO_SPARE_DIR = 'x' - CTRL_OFFSET, /* CTRL-x */

	/* Search keys */
	CFG_KEY_SEARCH_BWD = '\t', /* Tab */
	CFG_KEY_SEARCH_FWD = 13, /* Enter */
	CFG_KEY_SEARCH_DEL_CHAR = 127, /* Backspace */
};

/* The character that is drawn if there is no line on the row */
static const char cfg_no_line = '~';

/*
If no privilege to save the file, you can save it to this directory.

Should not contain '/' at the end
*/
static const char cfg_spare_save_dir[] = "/tmp";

/* Colors of displayed content */
static const struct color cfg_color_lines_fg = COLOR_NEW(192, 233, 233);
static const struct color cfg_color_stat_bg = COLOR_NEW(66, 165, 245);
static const struct color cfg_color_stat_fg = COLOR_NEW(245, 245, 245);

#endif /* _CFG_H */
