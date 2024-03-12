#include "err.h"
#include "raw_key.h"

char
raw_key_is_digit(int key)
{
	return '0' <= key && key <= '9';
}

unsigned char
raw_key_to_digit(int key)
{
	if (!raw_key_is_digit(key)) {
		err("Key %d is not a digit.", key);
	}
	return key - '0';
}
