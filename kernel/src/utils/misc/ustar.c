#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utils/lib.h>

#define USTAR_FILE_SIZE_OFFSET 124

static int oct2bin(unsigned char* str, int len) {
    int n = 0;
    unsigned char *c = str;
    while (len-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

void* ustar_lookup(void* archive, const char* filename, size_t* sz_out) {
    uint8_t* ptr = archive;
    while (!memcmp(ptr + 257, "ustar", 5)) {
        int filesize = oct2bin(ptr + USTAR_FILE_SIZE_OFFSET, 11);
        if (!memcmp(ptr, filename, strlen(filename) + 1)) {
            if (sz_out) *sz_out = filesize;
            return ptr + 512;
        }
        ptr += ALIGN_UP(filesize, 512) + 512;
    }
    return nullptr;
}
