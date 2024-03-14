#ifndef _MACROS_H
#define _MACROS_H

/* Repeats code several times. */
#define REPEAT(cnt, code) { \
	while (cnt-- > 0) { \
		(code); \
	} \
}

#endif /* _MACROS_H */
