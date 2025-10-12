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


#define PANIC_FLAGS_VECTOR (1 << 0)
#define PANIC_FLAGS_FRAME (1 << 1)
#define PANIC_FLAGS_ERROR (1 << 2)

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

    int scaleX = framebuffer->width / kernel_panic_image_data_width;
    int scaleY = framebuffer->height / kernel_panic_image_data_height;
    if (scaleX < 1) scaleX = 1;
    if (scaleY < 1) scaleY = 1;

    drawImage(kernel_panic_image, kernel_panic_image_data_width, kernel_panic_image_data_height, 0, 0, scaleX, scaleY);
    printString("The kernel is fucked dawg", 8, 8);

    char result[128];
    int y = 8;
    int x = 488;
    int i = 0;

    uint64_t *rbp_ptr;
    asm volatile ("mov %%rbp, %0" : "=r" (rbp_ptr));

    if (flags | PANIC_FLAGS_VECTOR) {
        switch (vector) {
            case INTERRUPT_HANDLER_DOUBLE_FAULT:
                if(flags | PANIC_FLAGS_FRAME) {
                    for (unsigned int j = 0; j < symbol_count; j++) {
                        if (frame->ip >= symbols[j].address && frame->ip  < symbols[j].address + symbols[j].size) {
                            uint64_t offset = frame->ip - symbols[j].address;
                            snprintf(result, sizeof(result), "%s: %lx + %lx", symbols[j].name, symbols[j].address, offset);
                            i++;
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
                            snprintf(result, sizeof(result), "%s: %lx + %lx", symbols[j].name, symbols[j].address, offset);
                            i++;
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
                            snprintf(result, sizeof(result), "%s: %lx + %lx", symbols[j].name, symbols[j].address, offset);
                            printString(result, x, y + i * 8);
                            i++;
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
                snprintf(result, sizeof(result), "%s: %lx + %lx", symbols[j].name, symbols[j].address, offset);
                printString(result, x, y + i * 8);
                i++;
                break;
            }
        }

        rbp_ptr = (uint64_t *)rbp_ptr[0];
    }

    y = 8;
    x = 250;
    i = 0;
    snprintf(result, sizeof(result), "%s", message);
    printString(result, 8, y + 8);
    snprintf(result, sizeof(result), "RAX=%lx", rax);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RBX=%lx", rbx);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RCX=%lx", rcx);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RDX=%lx", rdx);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RSI=%lx", rsi);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RDI=%lx", rdi);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RBP=%lx", rbp);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "RSP=%lx", rsp);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R8 =%lx", r8);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R9 =%lx", r9);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R10=%lx", r10);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R11=%lx", r11);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R12=%lx", r12);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R13=%lx", r13);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R14=%lx", r14);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "R15=%lx", r15);
    printString(result, x, y + i * 8);
    i++;
    i++;

    if (flags & PANIC_FLAGS_FRAME) {
        snprintf(result, sizeof(result), "IP=%lx", frame->ip);
        printString(result, x, y + i * 8);
        i++;
        snprintf(result, sizeof(result), "CS=%lx", frame->cs);
        printString(result, x, y + i * 8);
        i++;
        snprintf(result, sizeof(result), "FLAGS=%lx", frame->flags);
        printString(result, x, y + i * 8);
        i++;
        snprintf(result, sizeof(result), "SP=%lx", frame->sp);
        printString(result, x, y + i * 8);
        i++;
        snprintf(result, sizeof(result), "SS=%lx", frame->ss);
        printString(result, x, y + i * 8);
        i++;
        i++;
    }

    snprintf(result, sizeof(result), "RFLAGS=%lx", rflags);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "CR0=%lx", cr0);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "CR2=%lx", cr2);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "CR3=%lx", cr3);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "CR4=%lx", cr4);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "CR8=%lx", cr8);
    printString(result, x, y + i * 8);
    i++;
    i++;
    snprintf(result, sizeof(result), "GDTR Base=%lx", gdtr.base);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "GDTR Limit=%x", gdtr.limit);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "IDTR Base=%lx", idtr.base);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "IDTR Limit=%x", idtr.limit);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "LDT=%x", ldt);
    printString(result, x, y + i * 8);
    i++;
    snprintf(result, sizeof(result), "TR=%x", tr);
    printString(result, x, y + i * 8);
    i++;

    #ifndef MUTE_KERNEL_PANIC
        play_sound(1000);
    #endif

    while (1) {}
}
