#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <utils/defer.h>

#include <limine.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include <utils/locks/spinlock.h>

#include <drivers/16550uart.h>

#if defined (__x86_64__)
#include <arch/x86_64/intrin/portio.h>
#include <arch/x86_64/cpu/cpuid.h>
#elif defined (__riscv)
#include <arch/riscv64/sbi/console.h>
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
    if (ft_ctx != NULL) {
        if ((char)c == '\n')
            flanterm_write(ft_ctx, &cr, 1);
        flanterm_write(ft_ctx, (char*)&c, 1);
    }

    if (earlycon_serial.working) {
        if ((char)c == '\n')
            serial_send(&earlycon_serial, cr);
        serial_send(&earlycon_serial, c);
    }

    #if defined (__x86_64__)
    if (is_hypervisor)
        outb(0xE9, c);
    #elif defined (__riscv)
    sbi_console_putchar(c);
    #endif
}

int printf(const char* fmt, ...) {
    int lock1r = spinlock_lock(&stdio_spinlock);
    defer spinlock_unlock(&stdio_spinlock, lock1r);

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
    if (fb == NULL)
        return;
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
