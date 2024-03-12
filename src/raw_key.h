#ifndef _RAW_KEY_H
#define _RAW_KEY_H

/* Correspondence between an integer and a pressed raw key. */
typedef enum {
	/* TODO: try to place chars here using quotes */
	RAW_KEY_CTRL_Q = 17,
	RAW_KEY_CTRL_S = 19,
	RAW_KEY_ESC = 27,
	RAW_KEY_A = 97,
	RAW_KEY_D = 100,
	RAW_KEY_E = 101,
	RAW_KEY_H = 104,
	RAW_KEY_G = 103,
	RAW_KEY_I = 105,
	RAW_KEY_J = 106,
	RAW_KEY_K = 107,
	RAW_KEY_L = 108,
	RAW_KEY_Q = 113,
	RAW_KEY_S = 115,
	RAW_KEY_W = 119,
} RawKey;

char raw_key_is_digit(int);
unsigned char raw_key_to_digit(int);

#endif /* _RAW_KEY_H */
