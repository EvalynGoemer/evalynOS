#include <stdint.h>
#include <stdio.h>

extern const uint8_t __build_id_start[];
extern const uint8_t __build_id_end[];

void print_build_id() {
    const uint8_t* build_id_start = __build_id_start + 16;
    size_t size = __build_id_end - build_id_start;

    for (size_t i = 0; i < size; i += 2) {
        printf("%02x", build_id_start[i]);
    }
}

void print_build_info() {
    #if defined(__x86_64__)
    printf("Arch: x86-64; BuildID: ");
    #elif defined(__i386__)
    printf("Arch: i686; BuildID: ");
    #elif defined(__aarch64__)
    printf("Arch: aarch64; BuildID: ");
    #elif defined(__riscv) && __riscv_xlen == 64
    printf("Arch: riscv64; BuildID: ");
    #elif defined(__loongarch64)
    printf("Arch: loongarch64; BuildID: ");
    #else
    _Static_assert(0, "unimplemented");
    #endif

    print_build_id();
    printf("\n");
}
