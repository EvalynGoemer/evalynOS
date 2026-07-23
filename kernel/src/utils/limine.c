#include <stdint.h>
#include <limine.h>

[[gnu::used, gnu::section(".limine_requests_start")]]
volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

[[gnu::used, gnu::section(".limine_requests")]]
volatile uint64_t limine_base_revision[3] = LIMINE_BASE_REVISION(6);

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST_ID,
    .revision = 0,
#if defined(__x86_64__)
    .mode = LIMINE_PAGING_MODE_X86_64_5LVL,
    .min_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
    .max_mode = LIMINE_PAGING_MODE_X86_64_5LVL,
#elif defined(__loongarch64)
    .mode = LIMINE_PAGING_MODE_LOONGARCH_4LVL,
    .min_mode = LIMINE_PAGING_MODE_LOONGARCH_4LVL,
    .max_mode = LIMINE_PAGING_MODE_LOONGARCH_4LVL,
#elif defined(__riscv)
    .mode = LIMINE_PAGING_MODE_RISCV_SV48,
    .min_mode = LIMINE_PAGING_MODE_RISCV_SV39,
    .max_mode = LIMINE_PAGING_MODE_RISCV_SV57,
#elif defined(__aarch64__)
    .mode = LIMINE_PAGING_MODE_AARCH64_4LVL,
    .min_mode = LIMINE_PAGING_MODE_AARCH64_4LVL,
    .max_mode = LIMINE_PAGING_MODE_AARCH64_5LVL,
#endif
};

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_executable_file_request executable_file_request = {
    .id = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
    .revision = 0,
};

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0,
};

#if !defined(__x86_64__)
[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_dtb_request dtb_request = {
    .id = LIMINE_DTB_REQUEST_ID,
    .revision = 0
};
#endif

#if defined(__riscv)
[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_riscv_bsp_hartid_request riscv_bsp_hartid_request = {
    .id = LIMINE_RISCV_BSP_HARTID_REQUEST_ID,
    .revision = 0
};
#endif

[[gnu::used, gnu::section(".limine_requests_end")]]
volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;
