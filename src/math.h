#ifndef _MATH_H
#define _MATH_H

#include <stddef.h>

/* Divides two numbers, but rounds up if the division has a remainder. */
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Max value of `size_t` */
#define SIZE_MAX ((size_t)(-1))

#endif /* _MATH_H */
