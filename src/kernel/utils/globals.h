#pragma once

#define LIMINE_API_REVISION 3
#include <limine.h>

#include <stdint.h>

extern volatile uint64_t limine_base_revision[3];

extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_framebuffer *framebuffer;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_executable_address_request executable_address_request;
extern volatile struct limine_module_request module_request;

extern struct flanterm_context *ft_ctx;
