#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "res.h"

void
res_print(Res res)
{
	fprintf(stderr, "Result: %s\nerrno: %s.\n", res_str(res), strerror(errno));
}

char*
res_str(Res res)
{
	switch (res) {
	case RES_OK:
		return "Ok.";
	case RES_ERR_GET_ORIG_TERMIOS:
		return "Failed to get original termios parameters.";
	case RES_ERR_SET_ORIG_TERMIOS:
		return "Failed to set original termios parameters.";
	case RES_ERR_SET_RAW_TERMIOS:
		return "Failed to set raw termios parameters.";
	case RES_INVALID_ARGS_CNT:
		return "Invalid arguments count passed to program.";
	case RES_STDIN_NOT_REF_TO_TERM:
		return "Stdin does not refer to terminal. He's being redirected somewhere?";
	}
	return "Unknown.";
}
