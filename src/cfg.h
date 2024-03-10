#ifndef _CFG_H
#define _CFG_H

#include "raw_color.h"
#include "raw_key.h"

/* TODO: Fix it for another values */
#define CFG_TAB_SIZE (8)

/* Colors */
typedef enum {
	COLOR_STAT_BG = RAW_COLOR_BLUE,
	COLOR_STAT_FG = RAW_COLOR_BRIGHT_WHITE,
} Color;

/* Editor keys */
typedef enum {
	KEY_MODE_INS = RAW_KEY_I,
	KEY_MODE_NORM = RAW_KEY_ESC,
	KEY_MV_BEGIN_OF_F = RAW_KEY_W,
	KEY_MV_BEGIN_OF_ROW = RAW_KEY_A,
	KEY_MV_DOWN = RAW_KEY_J,
	KEY_MV_END_OF_F = RAW_KEY_S,
	KEY_MV_END_OF_ROW = RAW_KEY_D,
	KEY_MV_LEFT = RAW_KEY_H,
	KEY_MV_NEXT_TOK = RAW_KEY_E,
	KEY_MV_PREV_TOK = RAW_KEY_Q,
	KEY_MV_RIGHT = RAW_KEY_L,
	KEY_MV_UP = RAW_KEY_K,
	KEY_QUIT = RAW_KEY_CTRL_Q,
	KEY_SAVE = RAW_KEY_CTRL_S,
} Key;

#endif /* _CFG_H */
