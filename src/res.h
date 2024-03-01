#ifndef _RES_H
#define _RES_H

/* Result codes. */
typedef enum {
	RES_OK = 0,
	RES_ERR_GET_ORIG_TERMIOS,
	RES_ERR_SET_ORIG_TERMIOS,
	RES_ERR_SET_RAW_TERMIOS,
	RES_INVALID_ARGS_CNT,
	RES_STDIN_NOT_REF_TO_TERM,
} Res;

/* Prints result's and errno's understandable strings to stderr */
void res_print(Res);

/* Converts result to more understandable string. */
char *res_str(Res);

#endif /* _RES_H */
