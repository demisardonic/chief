#ifndef UTIL_H
#define UTIL_H

#define AUTHOR "MICKY LINDSAY"
#define VERSION "0.0"

#define CTRL_KEY(k) ((k) & 0x1f)
#define MIN(a, b) ((a < b ) ? a : b)
#define MAX(a, b) ((a < b ) ? b : a)
#define RANGE(n, a, b) (MAX(MIN(n, b), a))
void err(const char *error);

#endif
