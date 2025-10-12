#pragma once
#include <stdint.h>
#include <stddef.h>

extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);

extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, unsigned long n);

extern char *strcpy(char *strDest, const char *strSrc);
