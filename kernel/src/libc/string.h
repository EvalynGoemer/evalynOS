#pragma once

#include <stddef.h>

extern void* memcpy(void* restrict dest, const void* restrict src, size_t n);
extern void* memset(void* s, int c, size_t n);
extern void* memmove(void* dest, const void* src, size_t n);
extern int memcmp(const void* s1, const void* s2, size_t n);
extern void* memchr(const void *ptr, int value, size_t num);

extern size_t strlen(const char *str);
extern size_t strnlen(const char *str, size_t maxlen);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char* strcpy(char *dest, const char *src);
extern char* strchr(const char *s, int c);
extern char* strrchr(const char *s, int c);
