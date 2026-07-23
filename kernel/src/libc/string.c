#include <stdint.h>
#include <stddef.h>

#if defined(__x86_64__) || defined(__i386__)
void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    void* tmp = dest;
    asm volatile( "rep movsb" : "+D"(dest), "+S"(src), "+c"(n) : : "memory" );
    return tmp;
}

void* memset(void* dest, int c, size_t n) {
    void* tmp = dest;
    asm volatile("rep stosb" : "+D"(dest), "+c"(n) : "a"(c) : "memory");
    return tmp;
}
#else
void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return s;
}
#endif

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

void* memchr(const void *ptr, int value, size_t num) {
    const unsigned char *p = ptr;
    unsigned char c = (unsigned char)value;

    while (num--) {
        if (*p == c)
            return (void *)p;
        p++;
    }

    return NULL;
}

size_t strlen(const char *str) {
    const char *s = str;
    while (*s) {
        s++;
    }
    return s - str;
}

size_t strnlen(const char *str, size_t maxlen) {
    const char *s = str;
    while (maxlen && *s) {
        s++;
        maxlen--;
    }
    return s - str;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;

    while (i < n && s1[i] && (s1[i] == s2[i]))
        i++;
    if (i == n)
        return 0;

    return s1[i] - s2[i];
}

char* strcpy(char *dest, const char *src) {
    char *tmp = dest;
    while((*dest++ = *src++) != '\0');
    return tmp;
}

char* strchr(const char *s, int c) {
    while (true) {
        if (*s == (char)c)
            return (char*)s;
        if (*s++ == '\0')
            return NULL;
    }
}

char* strrchr(const char *s, int c) {
    const char *p = NULL;
    while (true) {
        if (*s == (char)c)
            p = s;
        if (*s++ == '\0')
            return (char*)p;
    }
}

