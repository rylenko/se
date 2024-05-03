#ifndef _DT_H
#define _DT_H

/*
 * Writes the date and time in the passed buffer using
 * `day-month-year_hour-minute-second` format. Make sure that buffer is big
 * enough.
 *
 * Returns 0 on success and -1 on error.
 */
int dt_str(char *, size_t);

#endif /* _DT_H */
