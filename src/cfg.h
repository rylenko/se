#ifndef _CFG_H
#define _CFG_H

#include "raw_color.h"
#include "raw_key.h"

/* If no privilege to save the file, you can save it to this directory */
static const char cfg_spare_save_dir[] = "/tmp";

/* Correspondence between raw colors and configurable editor colors. */
typedef enum {
	CFG_COLOR_STAT_BG = RAW_COLOR_BLUE,
	CFG_COLOR_STAT_FG = RAW_COLOR_BRIGHT_WHITE,
} CfgColor;

/* Correspondence between raw keys and configurable editor keys. */
typedef enum {
	/* Row management */
	CFG_KEY_DEL_ROW = RAW_KEY_CTRL_D,
	CFG_KEY_INS_ROW_BELOW = RAW_KEY_N,
	CFG_KEY_INS_ROW_TOP = RAW_KEY_CTRL_N,

	/* Modes switching */
	CFG_KEY_MODE_INS = RAW_KEY_I,
	CFG_KEY_MODE_NORM = RAW_KEY_ESC,

	/* Movement */
	CFG_KEY_MV_TO_BEGIN_OF_F = RAW_KEY_W,
	CFG_KEY_MV_TO_BEGIN_OF_ROW = RAW_KEY_A,
	CFG_KEY_MV_DOWN = RAW_KEY_J,
	CFG_KEY_MV_TO_END_OF_F = RAW_KEY_S,
	CFG_KEY_MV_TO_END_OF_ROW = RAW_KEY_D,
	CFG_KEY_MV_LEFT = RAW_KEY_H,
	CFG_KEY_MV_TO_NEXT_WORD = RAW_KEY_E,
	CFG_KEY_MV_TO_PREV_WORD = RAW_KEY_Q,
	CFG_KEY_MV_RIGHT = RAW_KEY_L,
	CFG_KEY_MV_UP = RAW_KEY_K,

	/* Content management */
	CFG_KEY_SAVE = RAW_KEY_CTRL_S,
	CFG_KEY_SAVE_TO_SPARE_DIR = RAW_KEY_CTRL_X,
	CFG_KEY_TRY_QUIT = RAW_KEY_CTRL_Q,
} CfgKey;

/* Other configurations */
typedef enum {
	CFG_ARROWS_ENABLED = 1,
	CFG_SPARE_SAVE_PATH_MAX_LEN = 256,
	/* TODO: Fix it for another values */
	CFG_TAB_SIZE = 8,
	CFG_QUIT_PRESSES_REM_AFT_CH = 3,
} Cfg;

#endif /* _CFG_H */
