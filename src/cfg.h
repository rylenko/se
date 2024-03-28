#ifndef _CFG_H
#define _CFG_H

#include "color.h"

/* If no privilege to save the file, you can save it to this directory */
static const char cfg_spare_save_dir[] = "/tmp";

/* Helpers for configuration. */
enum {
	CTRL_OFFSET = 96,
};

/* Different editor settings. */
typedef enum {
	CFG_DIRTY_FILE_QUIT_PRESSES_CNT = 4, /* Press to exit without saving */
	CFG_SPARE_PATH_MAX_LEN = 255, /* Max length of formatted spare save path */
	CFG_TAB_SIZE = 8, /* Count of spaces, which equals to one tab */
} Cfg;

/* Ascii keys to control the editor. */
typedef enum {
	/* Row management */
	CFG_KEY_DEL = 127, /* Backspace */
	CFG_KEY_DEL_ROW = 'd' - CTRL_OFFSET, /* CTRL-d */
	CFG_KEY_INS_BREAK = 13, /* Enter */
	CFG_KEY_INS_ROW_BELOW = 'n', /* n */
	CFG_KEY_INS_ROW_TOP = 'n' - CTRL_OFFSET, /* CTRL-n */

	/* Modes switching */
	CFG_KEY_MODE_INS = 'i', /* i */
	CFG_KEY_MODE_NORM = 27, /* Escape */

	/* Movement */
	CFG_KEY_MV_TO_BEGIN_OF_FILE = 'w', /* w */
	CFG_KEY_MV_TO_BEGIN_OF_ROW = 'a', /* a */
	CFG_KEY_MV_DOWN = 'j', /* j */
	CFG_KEY_MV_TO_END_OF_FILE = 's', /* s */
	CFG_KEY_MV_TO_END_OF_ROW = 'd', /* d */
	CFG_KEY_MV_LEFT = 'h', /* h */
	CFG_KEY_MV_TO_NEXT_WORD = 'e', /* e */
	CFG_KEY_MV_TO_PREV_WORD = 'q', /* q */
	CFG_KEY_MV_RIGHT = 'l', /* l */
	CFG_KEY_MV_UP = 'k', /* k */

	/* Save or quit */
	CFG_KEY_QUIT = 'q' - CTRL_OFFSET, /* CTRL-q */
	CFG_KEY_SAVE = 's' - CTRL_OFFSET, /* CTRL-s */
	CFG_KEY_SAVE_TO_SPARE_DIR = 'x' - CTRL_OFFSET, /* CTRL-x */
} CfgKey;

/* Colors of displayed content */
static const Color cfg_color_stat_bg = COLOR_NEW(66, 165, 245); /* Blue */
static const Color cfg_color_stat_fg = COLOR_NEW(245, 245, 245); /* White */

#endif /* _CFG_H */
