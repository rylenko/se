#ifndef _MACROS_H
#define _MACROS_H

/* Repeats code several times. */
#define REPEAT(cnt, code) { \
	size_t unpacked_cnt = cnt; \
	while (unpacked_cnt-- > 0) { \
		(code); \
	} \
}

#endif /* _MACROS_H */
