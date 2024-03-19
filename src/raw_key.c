/* assert */
#include <assert.h>
#include "raw_key.h"

char
raw_key_is_digit(const int key)
{
	return '0' <= key && key <= '9';
}

unsigned char
raw_key_to_digit(const int key)
{
	assert(raw_key_is_digit(key));
	return key - '0';
}
