#include <stdint.h>
#include <libc/stdio.h>
#include <utils/limine.h>

#include <arch/loongarch64/intrin/csr.h>
#include <arch/loongarch64/cpu/dmw.h>

static inline void* rebase_ptr(void *ptr, uint64_t delta) {
    if (!ptr)
        return NULL;
    return (void*)((uintptr_t)ptr + delta);
}

void rebase_limine_requests() {
    uint64_t old_hhdm = hhdm_request.response->offset;
    uint64_t cached_delta = HHDM_CACHED_OFFSET - old_hhdm;
    uint64_t weak_delta = HHDM_WEAK_UNCACHED_OFFSET - old_hhdm;

    struct limine_framebuffer_response *fbresp = framebuffer_request.response = rebase_ptr((void *)framebuffer_request.response, cached_delta);
    if (fbresp) {
        struct limine_framebuffer** buffers = fbresp->framebuffers = rebase_ptr(fbresp->framebuffers, cached_delta);
        uint64_t count = fbresp->framebuffer_count;

        for (uint64_t i = 0; i < count; i++) {
            struct limine_framebuffer* fb = buffers[i] = rebase_ptr(buffers[i], cached_delta);
            fb->address = rebase_ptr(fb->address, weak_delta);
            fb->edid = rebase_ptr(fb->edid, cached_delta);
            if (fb->modes) {
                struct limine_video_mode** modes = fb->modes = rebase_ptr(fb->modes, cached_delta);
                uint64_t mode_count = fb->mode_count;
                for (uint64_t j = 0; j < mode_count; j++)
                    modes[j] = rebase_ptr(modes[j], cached_delta);
            }
        }
    }

    memmap_request.response = rebase_ptr((void*)memmap_request.response, cached_delta);
    if (memmap_request.response) {
        memmap_request.response->entries = rebase_ptr(memmap_request.response->entries, cached_delta);
        for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
            memmap_request.response->entries[i] = rebase_ptr(memmap_request.response->entries[i], cached_delta);
    }

    executable_file_request.response = rebase_ptr((void*)executable_file_request.response, cached_delta);
    if (executable_file_request.response && executable_file_request.response->executable_file) {
        executable_file_request.response->executable_file = rebase_ptr(executable_file_request.response->executable_file, cached_delta);
        struct limine_file *file = executable_file_request.response->executable_file;
        file->address = rebase_ptr(file->address, cached_delta);
        file->path = rebase_ptr(file->path, cached_delta);
        file->string = rebase_ptr(file->string, cached_delta);
    }

    rsdp_request.response = rebase_ptr((void*)rsdp_request.response, cached_delta);
    if (rsdp_request.response)
        rsdp_request.response->address = rebase_ptr(rsdp_request.response->address, cached_delta);

    dtb_request.response = rebase_ptr((void*)dtb_request.response, cached_delta);
    if (dtb_request.response)
        dtb_request.response->dtb_ptr = rebase_ptr(dtb_request.response->dtb_ptr, cached_delta);

    hhdm_request.response = rebase_ptr((void*)hhdm_request.response, cached_delta);
    if (hhdm_request.response)
        hhdm_request.response->offset = HHDM_CACHED_OFFSET;

    paging_mode_request.response = rebase_ptr((void*)paging_mode_request.response, cached_delta);
    executable_address_request.response = rebase_ptr((void*)executable_address_request.response, cached_delta);
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
}
