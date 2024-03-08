#ifndef _CFG_H
#define _CFG_H

#include "raw_key.h"

/* TODO: Fix it for another values */
#define CFG_TAB_SIZE (8)

typedef enum {
	KEY_BEGIN_OF_F = RAW_KEY_W,
	KEY_BEGIN_OF_ROW = RAW_KEY_A,
	KEY_DOWN = RAW_KEY_J,
	KEY_END_OF_F = RAW_KEY_S,
	KEY_END_OF_ROW = RAW_KEY_D,
	KEY_INS_MODE = RAW_KEY_I,
	KEY_LEFT = RAW_KEY_H,
	KEY_NORM_MODE = RAW_KEY_ESC,
	KEY_QUIT = RAW_KEY_CTRL_Q,
	KEY_RIGHT = RAW_KEY_L,
	KEY_SAVE = RAW_KEY_CTRL_S,
	KEY_UP = RAW_KEY_K,
} Key;

#endif /* _CFG_H */
