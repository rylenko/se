#ifndef _DT_H
#define _DT_H

/*
Writes date and time using `day-month-year_hour-minute-second` format.

Returns 0 on success and -1 on error.
*/
int dt_str(char *, size_t);

#endif /* _DT_H */
