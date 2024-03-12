#ifndef _RAW_KEY_H
#define _RAW_KEY_H


/* Correspondence between an integer and a pressed raw key. */
typedef enum {
	RAW_KEY_A = 'a',
	RAW_KEY_CTRL_N = 14,
	RAW_KEY_CTRL_Q = 17,
	RAW_KEY_CTRL_S = 19,
	RAW_KEY_D = 'd',
	RAW_KEY_E = 'e',
	RAW_KEY_ESC = 27,
	RAW_KEY_G = 'g',
	RAW_KEY_H = 'h',
	RAW_KEY_I = 'i',
	RAW_KEY_J = 'j',
	RAW_KEY_K = 'k',
	RAW_KEY_L = 'l',
	RAW_KEY_N = 'n',
	RAW_KEY_Q = 'q',
	RAW_KEY_S = 's',
	RAW_KEY_W = 'w',
} RawKey;

char raw_key_is_digit(int);
unsigned char raw_key_to_digit(int);

#endif /* _RAW_KEY_H */
