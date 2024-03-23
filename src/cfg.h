#ifndef _CFG_H
#define _CFG_H

/* If no privilege to save the file, you can save it to this directory */
static const char cfg_spare_save_dir[] = "/tmp";

/* Different editor settings. */
typedef enum {
	CFG_DIRTY_QUIT_PRESSES_CNT = 4, /* Press to exit without saving */
	CFG_SPARE_SAVE_PATH_MAX_LEN = 256, /* Max length of full spare path */
	CFG_TAB_SIZE = 8, /* Count of spaces, which equals to one tab */
} Cfg;

/* Colors of displayed content. 256-color codes are used as colors. */
typedef enum {
	CFG_COLOR_STAT_BG = 25,
	CFG_COLOR_STAT_FG = 245,
} CfgColor;

/* Ascii keys to control the editor. */
typedef enum {
	/* Row management */
	CFG_KEY_DEL = 127, /* Backspace */
	CFG_KEY_DEL_ROW = 'd' - 96, /* CTRL-d */
	CFG_KEY_INS_BREAK = 13, /* Enter */
	CFG_KEY_INS_ROW_BELOW = 'n', /* n */
	CFG_KEY_INS_ROW_TOP = 'n' - 96, /* CTRL-n */

	/* Modes switching */
	CFG_KEY_MODE_INS = 'i', /* i */
	CFG_KEY_MODE_NORM = 27, /* Escape */

	/* Movement */
	CFG_KEY_MV_TO_BEGIN_OF_F = 'w', /* w */
	CFG_KEY_MV_TO_BEGIN_OF_ROW = 'a', /* a */
	CFG_KEY_MV_DOWN = 'j', /* j */
	CFG_KEY_MV_TO_END_OF_F = 's', /* s */
	CFG_KEY_MV_TO_END_OF_ROW = 'd', /* d */
	CFG_KEY_MV_LEFT = 'h', /* h */
	CFG_KEY_MV_TO_NEXT_WORD = 'e', /* e */
	CFG_KEY_MV_TO_PREV_WORD = 'q', /* q */
	CFG_KEY_MV_RIGHT = 'l', /* l */
	CFG_KEY_MV_UP = 'k', /* k */

	/* Content management */
	CFG_KEY_SAVE = 's' - 96, /* CTRL-s */
	CFG_KEY_SAVE_TO_SPARE_DIR = 'x' - 96, /* CTRL-x */
	CFG_KEY_TRY_QUIT = 'q' - 96, /* CTRL-q */
} CfgKey;

#endif /* _CFG_H */
