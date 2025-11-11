#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "libc/stdio.h"
#include "renderer/fb_renderer.h"
#include "renderer/kpanic_image.h"
#include "interupts/interupts.h"
#include "hardware/pcskpr.h"
#include "../generated/symbols.h"

struct descriptor_table_ptr {
    unsigned short limit;
    unsigned long base;
} __attribute__((packed));


static inline uint64_t read_tsc() {
    uint32_t hi, lo;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

uint64_t xorshift64star() {
    static uint64_t x = 0;
    if (x == 0) {
        x = read_tsc();
    }

    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

static int rand_between(int min, int max) {
    if (min > max) {
        int tmp = min;
        min = max;
        max = tmp;
    }

    uint64_t range = (uint64_t)(max - min + 1);
    uint64_t x, limit;

    limit = UINT64_MAX - (UINT64_MAX % range);

    do {
        x = xorshift64star();
    } while (x >= limit);

    return min + (x % range);
}

#define SIZEOF(arr) (sizeof(arr) / sizeof(*arr))
static const char *error_mssages[] = {
    "The kernel is fucked dawg",
    "The kernel got franned",
    "I like elephants",
    "Electric Boogaloo",
    "Everybody do the flop",
    "I like trains",
    "Hey, somebody kill me",
    "Miku, Miku, oooeeuuu",
    "Poteto Chips",
    "youtu.be/dQw4w9WgXcQ",
    "SHAWWWWWW",
    "GIT GUD",
    "Deltarune Tomorrow",
    "Better than XiOS",
    "Number Fifteen",
    "Talk is cheap",
    "How did we get here?",
    "Keyboard Cat!",
    "Prisencolinensinainciusol",
    "The Sun is a Deadly Lazer",
    "FUCK MICROSOFT",
    "MANKIND IS A FAILURE",
    "FREE WILL IS A FLAW",
    "THIS IS NOT OVER!",
    "Good Girl :3",
    "Albuquerque",
    "A Platypus?",
    "PERRY THE PLATYPUS!",
    "The Kernel-Panic-Inator",
    "My Name is Doof",
    "You got any bear claws?",
    "The panic revolving",
    "[[Hyperlink Blocked]]",
    "The Roaring Fraud",
    "The kernel is Glooby",
    "You heartless bastard!",
    "Aurora Borealis?",
    "My Kernel is ruined!",
    "Eat my shorts!",
    "Cromulently Ruined",
    "The kernel is a lie"
};

#define PANIC_FLAGS_VECTOR (1 << 0)
#define PANIC_FLAGS_FRAME (1 << 1)
#define PANIC_FLAGS_ERROR (1 << 2)

int panic_count = 0;

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void panic(char* message, int vector, struct interrupt_frame* frame, __attribute__((unused)) unsigned long error, int flags) {
    asm volatile("cli");

    unsigned long rax, rbx, rcx, rdx, rsi, rdi;
    unsigned long rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15;

    unsigned long rflags;
    unsigned long cr0, cr2, cr3, cr4, cr8;
    struct descriptor_table_ptr gdtr, idtr;
    unsigned short ldt, tr;

    asm volatile(
        "mov %%rax, %0\n\t"
        "mov %%rbx, %1\n\t"
        "mov %%rcx, %2\n\t"
        "mov %%rdx, %3\n\t"
        "mov %%rsi, %4\n\t"
        "mov %%rdi, %5\n\t"
        "mov %%rbp, %6\n\t"
        "mov %%rsp, %7\n\t"
        "mov %%r8,  %8\n\t"
        "mov %%r9,  %9\n\t"
        "mov %%r10, %10\n\t"
        "mov %%r11, %11\n\t"
        "mov %%r12, %12\n\t"
        "mov %%r13, %13\n\t"
        "mov %%r14, %14\n\t"
        "mov %%r15, %15\n\t"
        : "=m"(rax), "=m"(rbx), "=m"(rcx), "=m"(rdx), "=m"(rsi), "=m"(rdi), "=m"(rbp), "=m"(rsp), "=m"(r8), "=m"(r9), "=m"(r10), "=m"(r11), "=m"(r12), "=m"(r13), "=m"(r14), "=m"(r15));

    asm volatile(
        "pushfq\n\t"
        "pop %0\n\t"
        : "=r"(rflags)
    );

    asm volatile(
        "mov %%cr0, %0\n\t"
        "mov %%cr2, %1\n\t"
        "mov %%cr3, %2\n\t"
        "mov %%cr4, %3\n\t"
        "mov %%cr8, %4\n\t"
        : "=r"(cr0), "=r"(cr2), "=r"(cr3), "=r"(cr4), "=r"(cr8)
    );

    asm volatile(
        "sgdt %0\n\t"
        "sidt %1\n\t"
        : "=m"(gdtr), "=m"(idtr)
    );

    asm volatile(
        "sldt %0\n\t"
        "str %1\n\t"
        : "=m"(ldt), "=m"(tr)
    );

    stop_sound();
    if (panic_count > 0) {
        int scaleX = framebuffer->width / kernel_panic_image_data_width;
        int scaleY = framebuffer->height / kernel_panic_image_data_height;
        if (scaleX < 1) scaleX = 1;
        if (scaleY < 1) scaleY = 1;
        drawImage(kernel_panic_image, kernel_panic_image_data_width, kernel_panic_image_data_height, 0, 0, scaleX, scaleY);

        printString("The kernel is fucked 2: Electric Boogaloo (Panic Paniced)", 0, 0);
        printString(message, 100, 100);

        if(flags | PANIC_FLAGS_FRAME) {
            char result[32];
            snprintf(result, sizeof(result), "%lx", frame->ip);
            printString(result, 300, 300);
        }

        while (1) {

        }
    }
    panic_count++;

    printf("%s\n", error_mssages[rand_between(0, SIZEOF(error_mssages))]);

    printf("%s\n", message);

    printf("Register Dump:\n");

    printf("RAX=%lx\n", rax);
    printf("RBX=%lx\n", rbx);
    printf("RCX=%lx\n", rcx);
    printf("RDX=%lx\n", rdx);
    printf("RSI=%lx\n", rsi);
    printf("RDI=%lx\n", rdi);
    printf("RBP=%lx\n", rbp);
    printf("RSP=%lx\n", rsp);
    printf("R8 =%lx\n", r8);
    printf("R9 =%lx\n", r9);
    printf("R10=%lx\n", r10);
    printf("R11=%lx\n", r11);
    printf("R12=%lx\n", r12);
    printf("R13=%lx\n", r13);
    printf("R14=%lx\n", r14);
    printf("R15=%lx\n", r15);

    if (flags & PANIC_FLAGS_FRAME) {
        printf("IP=%lx\n", frame->ip);
        printf("CS=%lx\n", frame->cs);
        printf("FLAGS=%lx\n", frame->flags);
        printf("SP=%lx\n", frame->sp);
        printf("SS=%lx\n", frame->ss);
    }

    if (flags & PANIC_FLAGS_ERROR) {
        printf("ERR=%lx\n", error);
    }

    printf("RFLAGS=%lx\n", rflags);
    printf("CR0=%lx\n", cr0);
    printf("CR2=%lx\n", cr2);
    printf("CR3=%lx\n", cr3);
    printf("CR4=%lx\n", cr4);
    printf("CR8=%lx\n", cr8);

    printf("GDTR Base=%lx\n", gdtr.base);
    printf("GDTR Limit=%x\n", gdtr.limit);
    printf("IDTR Base=%lx\n", idtr.base);
    printf("IDTR Limit=%x\n", idtr.limit);
    printf("LDT=%x\n", ldt);
    printf("TR=%x\n", tr);

    printf("Stack Trace:\n");
    uint64_t *rbp_ptr;
    asm volatile ("mov %%rbp, %0" : "=r" (rbp_ptr));
    if (flags | PANIC_FLAGS_VECTOR) {
        switch (vector) {
            case INTERRUPT_HANDLER_DOUBLE_FAULT:
                if(flags | PANIC_FLAGS_FRAME) {
                    for (unsigned int j = 0; j < symbol_count; j++) {
                        if (frame->ip >= symbols[j].address && frame->ip  < symbols[j].address + symbols[j].size) {
                            uint64_t offset = frame->ip - symbols[j].address;
                            printf("%s: %lx + %lx\n", symbols[j].name, symbols[j].address, offset);
                            break;
                        }
                    }
                }
                break;
            case INTERRUPT_HANDLER_GENERAL_PROTECTION_FAULT:
                if(flags | PANIC_FLAGS_FRAME) {
                    for (unsigned int j = 0; j < symbol_count; j++) {
                        if (frame->ip >= symbols[j].address && frame->ip  < symbols[j].address + symbols[j].size) {
                            uint64_t offset = frame->ip - symbols[j].address;
                            printf("%s: %lx + %lx\n", symbols[j].name, symbols[j].address, offset);

                            break;
                        }
                    }
                }
                break;
            case INTERRUPT_HANDLER_PAGE_FAULT:
                if(flags | PANIC_FLAGS_FRAME) {
                    for (unsigned int j = 0; j < symbol_count; j++) {
                        if (frame->ip >= symbols[j].address && frame->ip  < symbols[j].address + symbols[j].size) {
                            uint64_t offset = frame->ip - symbols[j].address;
                            printf("%s: %lx + %lx\n", symbols[j].name, symbols[j].address, offset);
                            break;
                        }
                    }
                }
                break;
        }
    }

    while (rbp_ptr) {
        uint64_t rip = rbp_ptr[1];
        for (unsigned int j = 0; j < symbol_count; j++) {
            if (rip >= symbols[j].address && rip < symbols[j].address + symbols[j].size) {
                uint64_t offset = rip - symbols[j].address;
                printf("%s: %lx + %lx\n", symbols[j].name, symbols[j].address, offset);
                break;
            }
        }

        rbp_ptr = (uint64_t *)rbp_ptr[0];
    }

    printf("HALTING;\n");

    #ifndef MUTE_KERNEL_PANIC
        play_sound(1000);
    #endif

    while (1) {}
}
