#ifndef _COLOR_H
#define _COLOR_H

/*
 * RGB color.
 */
struct color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

/*
 * Macro to create new color in compile time.
 */
#define COLOR_NEW(red, green, blue) { .r = (red), .g = (green), .b = (blue) }

#endif /* _COLOR_H */
