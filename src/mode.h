#ifndef _MODE_H
#define _MODE_H

/* Editting modes. */
enum Mode {
	MODE_INS, /* Insert mode for text input */
	MODE_NORM, /* Normal mode for movement, number input, etc. */
	MODE_SEARCH, /* Text searching mode */
};

/* Converts mode to a string. `NULL` if no string for passed mode. */
char *mode_str(enum Mode);

#endif /* _MODE_H */
