#ifndef _MODE_H
#define _MODE_H

/*
 * Editting modes.
 */
enum mode {
	MODE_INS, /* Text inserting mode. */
	MODE_NORM, /* Normal mode for movement, number input, etc. */
	MODE_SEARCH, /* Text searching mode. */
};

/*
 * Converts mode to a string.
 *
 * Returns string even if mode is unknown.
 */
char *mode_str(enum mode);

#endif /* _MODE_H */
