#include <stdint.h>
#include <limine.h>
#include <arch/generic/paging/paging.h>
#include <utils/lib.h>

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

[[gnu::used, gnu::section(".limine_requests")]]
volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
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

uint64_t hhdm_offset = 0;
uint64_t kernel_virtual_base = 0;
uint64_t memmap_last_paddr = 0;
void*    initramfs = nullptr;
void*    kernel_elf_file = nullptr;
uint64_t kernel_elf_size = 0;

void limine_save_requests() {
    hhdm_offset = hhdm_request.response->offset;

    uint64_t memmap_end = 0;
    struct limine_memmap_response* mm = memmap_request.response;
    for (uint64_t i = 0; i < mm->entry_count; i++) {
        struct limine_memmap_entry* entry = mm->entries[i];
        if (entry->type == LIMINE_MEMMAP_RESERVED)   continue;
        if (entry->type == LIMINE_MEMMAP_BAD_MEMORY) continue;
        uint64_t entry_end = entry->base + entry->length;
        if (entry_end > memmap_end) memmap_end = entry_end;
    }
    memmap_last_paddr = ALIGN_UP(memmap_end, PAGE_SIZE);

    if (executable_file_request.response) {
        kernel_elf_file = executable_file_request.response->executable_file->address;
        kernel_elf_size = executable_file_request.response->executable_file->size;
    }

    if (executable_address_request.response)
        kernel_virtual_base = executable_address_request.response->virtual_base;

    if (module_request.response && module_request.response->module_count > 0)
        initramfs = module_request.response->modules[0]->address;
}
