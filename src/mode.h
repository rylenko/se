#ifndef _MODE_H
#define _MODE_H

/* Editting modes. */
typedef enum {
	MODE_INS, /* Insert mode for text input */
	MODE_NORM, /* Normal mode for movement, number input, etc. */
} Mode;

/* Converts mode to a string. `NULL` if no string for passed mode. */
char *mode_str(Mode);

#endif /* _MODE_H */
