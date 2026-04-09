#include <mem/pmm.h>
#include <arch/generic/panic.h>


uint64_t pmm_alloc_stub() {
    panic("PMM: Alloc called before PMM init");
    return 0;
}

void pmm_free_stub([[gnu::unused]] uint64_t phys) {
    panic("PMM: Free called before PMM init");
}

pmm_alloc_page_t pmm_alloc_page = pmm_alloc_stub;
pmm_free_page_t pmm_free_page = pmm_free_stub;
