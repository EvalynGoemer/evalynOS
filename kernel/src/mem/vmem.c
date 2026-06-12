// inspired by https://github.com/rdmsr/zag/blob/ff180bb385d3d706de73cad93f7d9c5baf791e59/src/mm/vmem.zig
// permission from rdmsr to use with MIT Licence here

#include <stddefer.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include <utils/lib.h>
#include <utils/defer.h>
#include <utils/limine.h>
#include <utils/dstruct/llist.h>
#include <utils/dstruct/bstree.h>
#include <utils/locks/ticketlock.h>

#include <mem/pmm.h>
#include <mem/vmem.h>
#include <mem/spalloc.h>

#include <arch/generic/panic.h>
#include <arch/generic/paging/paging.h>
#include <loader/elf_structs.h>

vmem_allocator_t kernel_vmem_allocator = {0};

void vmem_init() {
    vmem_allocator_init(&kernel_vmem_allocator, VADDR_HIGHER_HALF_BASE, (UINT64_MAX - VADDR_HIGHER_HALF_BASE) + 1, PAGE_SIZE);

    // remove HHDM from vmem
    struct limine_memmap_entry* last_entry = memmap_request.response->entries[memmap_request.response->entry_count - 1];
    uint64_t alloced = vmem_alloc(&kernel_vmem_allocator, last_entry->base + last_entry->length, hhdm_request.response->offset);
    assert(alloced == hhdm_request.response->offset);

    // remove kernel binary from vmem
    struct limine_executable_address_response *kaddr = executable_address_request.response;
    struct limine_executable_file_response *kexec = executable_file_request.response;
    struct elf_header_64 *header = (struct elf_header_64 *)kexec->executable_file->address;
    struct elf_program_header_64 *prog_headers = (struct elf_program_header_64 *)((uint8_t *)kexec->executable_file->address + header->program_header_table);

    uint64_t elf_base = (uint64_t)-1;
    uint64_t vstart = (uint64_t)-1;
    uint64_t vend = 0;

    for (uint16_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64 *ph = &prog_headers[i];
        if (ph->type != ELF_PROG_PT_LOAD_TYPE) continue;
        if (ph->virt_addr < elf_base)
            elf_base = ph->virt_addr;
        uint64_t vbase = ph->virt_addr + kaddr->virtual_base - elf_base;
        uint64_t vend_local = vbase + ph->mem_size;
        if (vbase < vstart)
            vstart = vbase;
        if (vend_local > vend)
            vend = vend_local;
    }

    if (elf_base == (uint64_t)-1 || vstart == (uint64_t)-1)
        panic("Unable to get kernel virtual range from elf");

    alloced = vmem_alloc(&kernel_vmem_allocator, vend - vstart, vstart);
    assert(alloced == vstart);

    LOG_TAGGED_OK("MEMORY", ANSI_BGREEN, "Kernel VMEM Allocator Init")
}

static uint64_t vmem_segment_get_value(bstree_node_t* node) {
    vmem_segment_t* n = CONTAINER_OF(node, vmem_segment_t, segment_tree_node);
    return n->base;
}

void vmem_allocator_init(vmem_allocator_t* alloc, uint64_t base, uint64_t size, uint32_t quantum) {
    alloc->base = base;
    alloc->size = size;
    alloc->quantum = quantum;

    alloc->segments_tree = BSTREE_INIT;
    alloc->segments_tree.type = BST_TYPE_RB;
    alloc->segments_tree.value_of_node = vmem_segment_get_value;

    alloc->segments_list = LLIST_INIT;
    for (uint32_t i = 0; i < ARRAY_SIZE(alloc->freelists); i++) {
        alloc->freelists[i] = LLIST_INIT;
    }

    bool status = spalloc_init(&alloc->segment_allocator, sizeof(vmem_segment_t), alignof(vmem_segment_t));
    assert(status == true);

    vmem_add_segment(alloc, base, size);
}

void vmem_add_segment(vmem_allocator_t* alloc, uint64_t base, uint64_t size) {
    int lock1r = ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock, lock1r);

    vmem_segment_t* seg = spalloc_malloc(&alloc->segment_allocator);
    memset(seg, 0, sizeof(vmem_segment_t));
    seg->base = base;
    seg->size = size;

    llist_node_t* cnode = alloc->segments_list.head;
    while (cnode != nullptr) {
        vmem_segment_t* cseg = CONTAINER_OF(cnode, vmem_segment_t, segment_list_node);
        if (cseg->base > base) break;
        cnode = cnode->next;
    }

    if (cnode)
        llist_node_prepend(&alloc->segments_list, cnode, &seg->segment_list_node);
    else
        llist_push_back(&alloc->segments_list, &seg->segment_list_node);

    llist_push(&alloc->freelists[log2ull(size)], &seg->freelist_node);
}

struct fit_alloc_ret {
    uint64_t addr;
    int32_t flist_idx;
    vmem_segment_t* seg;
};

static struct fit_alloc_ret fit_alloc(vmem_allocator_t* alloc, uint64_t size, uint64_t addr) {
    // either a fixed allocation or an open allocation
    uint64_t min = ALIGN_UP(addr, alloc->quantum);
    uint64_t max = addr ? ALIGN_UP(addr + size, alloc->quantum) : UINT64_MAX;

    uint32_t index = log2ull(size);
    assert(index < ARRAY_SIZE(alloc->freelists));

    for (uint32_t i = index; i < ARRAY_SIZE(alloc->freelists); i++) {
        LLIST_FOR_EACH(alloc->freelists[i], cnode) {
            vmem_segment_t* cseg = CONTAINER_OF(cnode, vmem_segment_t, freelist_node);
            uint64_t alloc_start = MAX(min, cseg->base);
            alloc_start = ALIGN_UP(alloc_start, alloc->quantum);

            if (alloc_start < min)
                continue;
            if (cseg->size < size)
                continue;
            if (alloc_start - cseg->base > cseg->size - size)
                continue;
            if (max != UINT64_MAX && alloc_start + size > max)
                continue;

            return (struct fit_alloc_ret){.addr = alloc_start, .flist_idx = i, .seg = cseg};
        }
    }

    return (struct fit_alloc_ret){.addr = 0, .flist_idx = -1, .seg = nullptr};
}

// addr 0 means any address
uint64_t vmem_alloc(vmem_allocator_t* alloc, uint64_t size, uint64_t addr) {
    int lock1r = ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock, lock1r);

    size = ALIGN_UP(size, alloc->quantum);
    if (!IS_ALIGNED(addr, alloc->quantum))
        return 0;

    struct fit_alloc_ret ret = fit_alloc(alloc, size, addr);
    if (ret.seg == nullptr)
        return 0;

    if (addr != 0)
        assert(ret.addr == addr);

    assert(ret.seg->size >= size);

    llist_node_delete(&alloc->freelists[ret.flist_idx], &ret.seg->freelist_node);

    // left split
    if (ret.seg->base != ret.addr && (ret.addr - ret.seg->base) >= alloc->quantum) {
        vmem_segment_t* new_seg = spalloc_malloc(&alloc->segment_allocator);
        memset(new_seg, 0, sizeof(vmem_segment_t));
        new_seg->base = ret.seg->base;
        new_seg->size = ret.addr - ret.seg->base;

        llist_node_prepend(&alloc->segments_list, &ret.seg->segment_list_node, &new_seg->segment_list_node);
        llist_push(&alloc->freelists[log2ull(new_seg->size)], &new_seg->freelist_node);

        ret.seg->base = ret.addr;
        ret.seg->size -= new_seg->size;
    }

    // right split
    if (ret.seg->size != size && (ret.seg->size - size) >= alloc->quantum) {
        vmem_segment_t* new_seg = spalloc_malloc(&alloc->segment_allocator);
        memset(new_seg, 0, sizeof(vmem_segment_t));
        new_seg->allocated = true;
        new_seg->base = ret.seg->base;
        new_seg->size = size;

        ret.seg->base += size;
        ret.seg->size -= size;
        ret.seg->allocated = false;
        ret.seg->freelist_node = (llist_node_t){0};

        llist_node_prepend(&alloc->segments_list, &ret.seg->segment_list_node, &new_seg->segment_list_node);
        llist_push(&alloc->freelists[log2ull(ret.seg->size)], &ret.seg->freelist_node);

        bstree_insert(&alloc->segments_tree, &new_seg->segment_tree_node);

        return new_seg->base;
    }

    // no more splitting needed
    ret.seg->allocated = true;
    ret.seg->segment_tree_node = BSTREE_NODE_INIT;
    bstree_insert(&alloc->segments_tree, &ret.seg->segment_tree_node);

    return ret.seg->base;
}

static vmem_segment_t* vmem_find_segment_nolock(vmem_allocator_t* alloc, uint64_t addr) {
    bstree_node_t* bnode = bstree_search(&alloc->segments_tree, addr, BST_SEARCH_TYPE_NEAREST_LTE);
    if (!bnode)
        return nullptr;

    vmem_segment_t* seg = CONTAINER_OF(bnode, vmem_segment_t, segment_tree_node);
    if (addr < seg->base || addr - seg->base >= seg->size)
        return nullptr;

    return seg;
}

void vmem_free(vmem_allocator_t* alloc, uint64_t addr, uint64_t size) {
    int lock1r = ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock, lock1r);

    size = ALIGN_UP(size, alloc->quantum);
    addr = ALIGN_UP(addr, alloc->quantum);
    if (size == 0)
        return;

    // handle cross segment frees
    uint64_t cur_addr = addr;
    uint64_t end_addr = addr + size;

    while (cur_addr < end_addr) {
        vmem_segment_t* seg = vmem_find_segment_nolock(alloc, cur_addr);
        if (seg == nullptr) return;

        uint64_t seg_end = seg->base + seg->size;

        if (!seg->allocated) {
            cur_addr = seg->base + seg->size;
            continue;
        }

        bstree_remove(&alloc->segments_tree, &seg->segment_tree_node);

        // left split
        if (seg->base < cur_addr) {
            vmem_segment_t* left_seg = spalloc_malloc(&alloc->segment_allocator);
            memset(left_seg, 0, sizeof(vmem_segment_t));
            left_seg->base = seg->base;
            left_seg->size = cur_addr - seg->base;
            left_seg->allocated = true;

            seg->base = cur_addr;
            seg->size -= left_seg->size;

            llist_node_prepend(&alloc->segments_list, &seg->segment_list_node, &left_seg->segment_list_node);
            bstree_insert(&alloc->segments_tree, &left_seg->segment_tree_node);
        }

        // right split
        if (seg_end > end_addr) {
            vmem_segment_t* right_seg = spalloc_malloc(&alloc->segment_allocator);
            memset(right_seg, 0, sizeof(vmem_segment_t));
            right_seg->base = end_addr;
            right_seg->size = seg_end - end_addr;
            right_seg->allocated = true;

            seg->size = end_addr - seg->base;

            llist_node_append(&alloc->segments_list, &seg->segment_list_node, &right_seg->segment_list_node);
            bstree_insert(&alloc->segments_tree, &right_seg->segment_tree_node);
        }

        cur_addr = MIN(seg_end, end_addr);

        seg->allocated = false;

        // left coalesce
        llist_node_t* prev_node = seg->segment_list_node.prev;
        if (prev_node != nullptr) {
            vmem_segment_t* prev_seg = CONTAINER_OF(prev_node, vmem_segment_t, segment_list_node);
            if (!prev_seg->allocated && prev_seg->base + prev_seg->size == seg->base) {
                llist_node_delete(&alloc->freelists[log2ull(prev_seg->size)], &prev_seg->freelist_node);
                llist_node_delete(&alloc->segments_list, &prev_seg->segment_list_node);
                seg->base = prev_seg->base;
                seg->size += prev_seg->size;
                spalloc_free(&alloc->segment_allocator, prev_seg);
            }
        }

        // right coalesce
        llist_node_t* next_node = seg->segment_list_node.next;
        if (next_node != nullptr) {
            vmem_segment_t* next_seg = CONTAINER_OF(next_node, vmem_segment_t, segment_list_node);
            if (!next_seg->allocated && seg->base + seg->size == next_seg->base) {
                llist_node_delete(&alloc->freelists[log2ull(next_seg->size)], &next_seg->freelist_node);
                llist_node_delete(&alloc->segments_list, &next_seg->segment_list_node);
                seg->size += next_seg->size;
                spalloc_free(&alloc->segment_allocator, next_seg);
            }
        }

        assert(seg->size != 0);
        seg->freelist_node = (llist_node_t){0};
        llist_push(&alloc->freelists[log2ull(seg->size)], &seg->freelist_node);
    }
}

vmem_segment_t* vmem_find_segment(vmem_allocator_t* alloc, uint64_t addr) {
    int lock1r = ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock, lock1r);
    return vmem_find_segment_nolock(alloc, addr);
}
