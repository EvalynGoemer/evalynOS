#pragma once
#include <stdint.h>

#define LIMINE_API_REVISION 3
#include <limine.h>

extern volatile struct limine_framebuffer *framebuffer;
extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_executable_address_request executable_address_request;
