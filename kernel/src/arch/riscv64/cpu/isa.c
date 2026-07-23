#include <string.h>

bool isa_has_baseline(const char* isa, const char* required) {
    if (strncmp(isa, required, 4) == 0) {
        isa += 4;
        required += 4;
    } else {
        return false;
    }

    const char *end = strchr(isa, '_');
    size_t size = end ? (end - isa) : strlen(isa);

    while (*required) {
        if (memchr(isa, *required, size) == NULL)
            return false;
        required++;
    }

    return true;
}

bool isa_has_extension(const char *isa, const char *ext) {
    const char *p = strchr(isa, '_');
    if (!p)
        return false;

    size_t ext_len = strlen(ext);

    while (*p) {
        p++;

        const char *end = strchr(p, '_');
        size_t len = end ? (end - p) : strlen(p);

        if (ext_len == len && strncmp(p, ext, len) == 0)
            return true;

        if (!end)
            break;

        p = end;
    }

    return false;
}
