#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(__x86_64__) || defined(__i386__)
    extern void* memcpy(void* dst, const void* src, size_t n);
    extern void* memset(void* dst, int c, size_t n);
#else
    extern void* memcpy(void* restrict dest, const void* restrict src, size_t n);
    extern void* memset(void* s, int c, size_t n);
#endif

extern void* memmove(void* dest, const void* src, size_t n);
extern int memcmp(const void* s1, const void* s2, size_t n);
