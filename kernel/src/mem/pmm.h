#pragma once
#include <stdint.h>

#define PAGE_SIZE 4096

typedef uint64_t (*pmm_alloc_page_t)(void);
typedef void (*pmm_free_page_t)(uint64_t);

extern pmm_alloc_page_t pmm_alloc_page;
extern pmm_free_page_t pmm_free_page;
