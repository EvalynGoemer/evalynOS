#include <stdint.h>
#include <stddef.h>
#include <utils/lib.h>

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    // on x86 do a rep movsb if copying 2kb+
    #if defined(__x86_64__) || defined(__i386__)
    if (n >= 2048) {
        void* tmp = dest;
        asm volatile( "rep movsb" : "+D"(dest), "+S"(src), "+c"(n) : : "memory" );
        return tmp;
    }
    #endif

    uint8_t* restrict pdest = (uint8_t* restrict)dest;
    const uint8_t* restrict psrc = (const uint8_t* restrict)src;

    // if copying a small amount do a byte loop
    if (n < sizeof(size_t)) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
        return dest;
    }

    // if dest and src can be aligned do a word copy
    if (MISALIGNMENT(pdest, sizeof(size_t)) == MISALIGNMENT(psrc, sizeof(size_t))) {
        // align dest and src
        while (!IS_ALIGNED(pdest, sizeof(size_t))) {
            *pdest++ = *psrc++;
            n--;
        }

        size_t* restrict wdest = (size_t* restrict)pdest;
        const size_t* restrict wsrc = (const size_t* restrict)psrc;

        // do bulk word copies if possible
        while (n >= (8 * sizeof(size_t))) {
            wdest[0] = wsrc[0];
            wdest[1] = wsrc[1];
            wdest[2] = wsrc[2];
            wdest[3] = wsrc[3];
            wdest[4] = wsrc[4];
            wdest[5] = wsrc[5];
            wdest[6] = wsrc[6];
            wdest[7] = wsrc[7];
            wdest += 8;
            wsrc += 8;
            n -= (8 * sizeof(size_t));
        }

        // finish remaining words
        while (n >= sizeof(size_t)) {
            *wdest++ = *wsrc++;
            n -= sizeof(size_t);
        }

        pdest = (uint8_t* restrict)wdest;
        psrc = (const uint8_t* restrict)wsrc;
    }

    // final per byte copy
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void* memset(void* dest, int c, size_t n) {
    // on x86 do a rep stosb if setting more than 2kb
    #if defined(__x86_64__) || defined(__i386__)
    if (n >= 2048) {
        void* tmp = dest;
        asm volatile("rep stosb" : "+D"(dest), "+c"(n) : "a"(c) : "memory");
        return tmp;
    }
    #endif

    uint8_t* pdest = (uint8_t*)dest;

    // if copying a small amount do a byte loop
    if (n < sizeof(size_t)) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = c;
        }
        return dest;
    }

    // align pdest
    while (!IS_ALIGNED(pdest, sizeof(size_t))) {
        *pdest++ = c;
        n--;
    }

    size_t* wdest = (size_t*)pdest;

    size_t pattern = (unsigned char)c;
    pattern *= (~(size_t)0) / 0xFF;

    // do bulk word writes if possible
    while (n >= (8 * sizeof(size_t))) {
        wdest[0] = pattern;
        wdest[1] = pattern;
        wdest[2] = pattern;
        wdest[3] = pattern;
        wdest[4] = pattern;
        wdest[5] = pattern;
        wdest[6] = pattern;
        wdest[7] = pattern;
        wdest += 8;
        n -= (8 * sizeof(size_t));
    }

    // finish remaining words
    while (n >= sizeof(size_t)) {
        wdest[0] = pattern;
        wdest++;
        n -= sizeof(size_t);
    }

    pdest = (uint8_t*)wdest;

    // finish remaining bytes
    for (size_t i = 0; i < n; i++) {
        pdest[i] = (uint8_t)c;
    }

    return dest;
}

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

