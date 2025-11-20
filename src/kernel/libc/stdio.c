#include <utils/globals.h>
#include <drivers/x86_64/serial.h>

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include <flanterm.h>

#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_ALT_FORM_FLAG 1
#include <nanoprintf.h>

int printf(const char* fmt, ...) {
    char buf[256] = {'\0'};
    va_list args;
    va_start(args, fmt);
    npf_vsnprintf(buf, 256, fmt, args);
    va_end(args);

    flanterm_write(ft_ctx, buf, 256);

    if(serial_works) {
        write_serial(buf, 256);
    }

    return 0;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    npf_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return 0;
}
