#pragma once

#include <stdint.h>
#include <limine.h>

extern volatile uint64_t limine_requests_start_marker[];

extern volatile uint64_t limine_base_revision[];
extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_paging_mode_request paging_mode_request;
extern volatile struct limine_executable_address_request executable_address_request;
extern volatile struct limine_executable_file_request executable_file_request;
extern volatile struct limine_rsdp_request rsdp_request;

#if !defined(__x86_64__)
extern volatile struct limine_dtb_request dtb_request;
#endif

#if defined(__riscv)
extern volatile struct limine_riscv_bsp_hartid_request riscv_bsp_hartid_request;
#endif

extern volatile uint64_t limine_requests_end_marker[];

