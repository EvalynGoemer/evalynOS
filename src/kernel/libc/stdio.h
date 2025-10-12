#pragma once

#define TERM_WIDTH 80
#define TERM_HEIGHT 24
#include <stddef.h>

extern char terminal[TERM_HEIGHT][TERM_WIDTH];
extern int term_updated;
extern int printf(const char *fmt, ...);
extern int snprintf(char *str, size_t n, const char *fmt, ...);
extern void cleark();
