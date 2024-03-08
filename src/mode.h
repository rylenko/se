#ifndef _MODE_H
#define _MODE_H

/* Editting mode. */
typedef enum {
	MODE_INS,
	MODE_NORM,
} Mode;

/* Converts mode to a string. */
char *mode_str(Mode);

#endif /* _MODE_H */
