#include <stdint.h>
#include <libc/stdio.h>
#include <utils/limine.h>

#include <arch/loongarch64/intrin/csr.h>
#include <arch/loongarch64/cpu/dmw.h>

static inline void* rebase_ptr(void *ptr, uint64_t old_hhdm, uint64_t new_hhdm) {
    if (!ptr)
        return NULL;

    uintptr_t addr = (uintptr_t)ptr;
    return (void *)(addr + (new_hhdm - old_hhdm));
}

static void rebase_limine_requests() {
    uint64_t old_hhdm = hhdm_request.response->offset;

    framebuffer_request.response = rebase_ptr((void *)framebuffer_request.response, old_hhdm, HHDM_CACHED_OFFSET);
    if (framebuffer_request.response) {
        framebuffer_request.response->framebuffers = rebase_ptr(framebuffer_request.response->framebuffers, old_hhdm, HHDM_CACHED_OFFSET);
        for (uint64_t i = 0; i < framebuffer_request.response->framebuffer_count; i++) {
            framebuffer_request.response->framebuffers[i] = rebase_ptr(framebuffer_request.response->framebuffers[i], old_hhdm, HHDM_CACHED_OFFSET);
            struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[i];
            fb->address = rebase_ptr(fb->address, old_hhdm, HHDM_WEAK_UNCACHED_OFFSET);
            fb->edid = rebase_ptr(fb->edid, old_hhdm, HHDM_CACHED_OFFSET);
            if (fb->modes) {
                fb->modes = rebase_ptr(fb->modes, old_hhdm, HHDM_CACHED_OFFSET);
                for (uint64_t j = 0; j < fb->mode_count; j++) {
                    fb->modes[j] = rebase_ptr(fb->modes[j], old_hhdm, HHDM_CACHED_OFFSET);
                }
            }
        }
    }

    memmap_request.response = rebase_ptr((void *)memmap_request.response, old_hhdm, HHDM_CACHED_OFFSET);
    if (memmap_request.response) {
        memmap_request.response->entries = rebase_ptr(memmap_request.response->entries, old_hhdm, HHDM_CACHED_OFFSET);
        for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
            memmap_request.response->entries[i] = rebase_ptr(memmap_request.response->entries[i], old_hhdm, HHDM_CACHED_OFFSET);
    }

    executable_file_request.response = rebase_ptr((void *)executable_file_request.response, old_hhdm, HHDM_CACHED_OFFSET);
    if (executable_file_request.response && executable_file_request.response->executable_file) {
        executable_file_request.response->executable_file = rebase_ptr(executable_file_request.response->executable_file, old_hhdm, HHDM_CACHED_OFFSET);
        struct limine_file *file = executable_file_request.response->executable_file;
        file->address = rebase_ptr(file->address, old_hhdm, HHDM_CACHED_OFFSET);
        file->path = rebase_ptr(file->path, old_hhdm, HHDM_CACHED_OFFSET);
        file->string = rebase_ptr(file->string, old_hhdm, HHDM_CACHED_OFFSET);
    }

    rsdp_request.response = rebase_ptr((void *)rsdp_request.response, old_hhdm, HHDM_CACHED_OFFSET);
    if (rsdp_request.response)
        rsdp_request.response->address = rebase_ptr(rsdp_request.response->address, old_hhdm, HHDM_CACHED_OFFSET);

    hhdm_request.response = rebase_ptr((void *)hhdm_request.response, old_hhdm, HHDM_CACHED_OFFSET);
    if (hhdm_request.response)
        hhdm_request.response->offset = HHDM_CACHED_OFFSET;

    paging_mode_request.response = rebase_ptr((void *)paging_mode_request.response, old_hhdm, HHDM_CACHED_OFFSET);
    executable_address_request.response = rebase_ptr((void *)executable_address_request.response, old_hhdm, HHDM_CACHED_OFFSET);
}

void setup_dmw() {
    uint64_t dmw_val;

    // norm cached
    dmw_val = (0x8ull << DMW_VSEG_SHIFT) | DMW_MAT_CACHED | DMW_PLV0;
    csrwr(CSR_DMW0, dmw_val);

    // set DMW1 to mirror DMW0 to not have undefined DMWs (normal caching)
    dmw_val = (0x9ull << DMW_VSEG_SHIFT) | DMW_MAT_CACHED | DMW_PLV0;
    csrwr(CSR_DMW1, dmw_val);

    // weakly ordered uncached
    dmw_val = (0xAull << DMW_VSEG_SHIFT) | DMW_MAT_WEAK_UNCACHED | DMW_PLV0;
    csrwr(CSR_DMW2, dmw_val);

    // stongly ordered uncached
    dmw_val = (0xBull << DMW_VSEG_SHIFT) | DMW_MAT_STRONG_UNCACHED | DMW_PLV0;
    csrwr(CSR_DMW3, dmw_val);

    asm ("dbar 0" ::: "memory");
    asm ("ibar 0" ::: "memory");

    rebase_limine_requests();
}
