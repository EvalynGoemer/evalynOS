#pragma once

#include <stdint.h>
#include <limine.h>

extern volatile uint64_t limine_requests_start_marker[];

extern volatile uint64_t limine_base_revision[];
extern volatile struct limine_framebuffer_request framebuffer_request;

extern volatile uint64_t limine_requests_end_marker[];

