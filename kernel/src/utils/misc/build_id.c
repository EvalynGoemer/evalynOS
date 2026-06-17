#include <utils/lib.h>
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
    printf("Arch: " STRINGIFY(TARGET_ARCH) "; BuildID: ");

    print_build_id();
    printf("\n");
}
