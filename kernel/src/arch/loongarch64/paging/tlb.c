#include <stdint.h>
#include <utils/limine.h>
#include <assert.h>
#include <arch/loongarch64/intrin/csr.h>
#include <arch/loongarch64/paging/tlb.h>

void setup_tlb_refill_handler() {
    uint64_t handler_vaddr = (uint64_t)tlb_refill_handler;
    uint64_t vbase = executable_address_request.response->virtual_base;
    uint64_t pbase = executable_address_request.response->physical_base;
    uint64_t handler_paddr = handler_vaddr - vbase + pbase;
    assert(handler_paddr % 4096 == 0);
    csrwr(CSR_TLBRENTRY, handler_paddr);
}
