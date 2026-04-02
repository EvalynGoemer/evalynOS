#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include <limine.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <utils/locks/spinlock.h>

#if defined (__x86_64__)
#include <arch/x86_64/drivers/16550uart.h>
#include <arch/x86_64/drivers/portio.h>
#endif

#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS   1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS       0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS       1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS      1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS   1
#define NANOPRINTF_USE_ALT_FORM_FLAG                 1
#include <nanoprintf.h>

// align to a cache line
[[gnu::aligned(64)]] spinlock_t stdio_spinlock = {0};
struct flanterm_context *ft_ctx;

static const char cr = '\r';
void internal_putc(int c, [[gnu::unused]] void *_) {
    int lock1r = spinlock_lock(&stdio_spinlock);
    if ((char)c == '\n')
        flanterm_write(ft_ctx, &cr, 1);
    flanterm_write(ft_ctx, (char*)&c, 1);
    spinlock_unlock(&stdio_spinlock, lock1r);

    #if defined (__x86_64__)
    if (serial_works) {
        if ((char)c == '\n')
            serial_send(cr);
        serial_send(c);
    }
    outb(0xE9, c);
    #endif
}

int printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = npf_vpprintf(internal_putc, NULL, fmt, args);
    va_end(args);
    return ret;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    npf_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return 0;
}

void stdio_init(struct limine_framebuffer* fb) {
    ft_ctx = flanterm_fb_init(
        NULL, NULL,
        fb->address, fb->width, fb->height, fb->pitch,
        fb->red_mask_size,   fb->red_mask_shift,
        fb->green_mask_size, fb->green_mask_shift,
        fb->blue_mask_size,  fb->blue_mask_shift,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, 0, 0, 1, 0, 0, 0, 0
    );
}
