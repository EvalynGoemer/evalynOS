#include <utils/locks/ticketlock.h>
#include <arch/intrin/paging.h>
#include <mem/pfndb.h>
#include <arch/generic/panic.h>
#include <assert.h>
#include <stdint.h>
#include <mem/balloc.h>
#include <mem/buddy.h>
#include <utils/lib.h>
#include <utils/defer.h>
#include <utils/limine.h>
#include <utils/dstruct/llist.h>
#include <math.h>
#include <stdio.h>

#define GET_ORDER_SIZE(order) (1ull << (PAGE_SIZE_SHIFT + (order)))
#define GET_ORDER(addr) ((__builtin_ctzll((addr)) - PAGE_SIZE_SHIFT))
#define SET_MAX_ORDER(order, val) ((order) = (((order) & 0x0f) | (val) << 4))
#define SET_CUR_ORDER(order, val) ((order) = (((order) & 0xf0) | (val) << 0))
#define GET_MAX_ORDER(order) (((order) >> 4) & 0xf)
#define GET_CUR_ORDER(order) (((order) >> 0) & 0xf)

static ticketlock_t lock = {0};
static llist_t orders[MAX_ORDER + 1] = {0};

static inline void buddy_add_block(uint64_t paddr, int order) {
    page_t* entry = pfndb_get_entry(paddr);
    entry->type = PFNDB_PAGE_FREE;
    SET_MAX_ORDER(entry->order, order);
    SET_CUR_ORDER(entry->order, order);
    llist_push(&orders[order], (llist_node_t*)TO_HHDM(paddr));
}

void buddy_init() {
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t start = MAX(ALIGN_UP(entry->base, PAGE_SIZE), balloc_last_addr + PAGE_SIZE);
        uint64_t end   = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);
        if (end <= start) continue;

        uint64_t addr = start;
        while (addr < end) {
            uint64_t pages = (end - addr) >> PAGE_SIZE_SHIFT;
            int max_fit = log2ull(pages);
            int align   = GET_ORDER(addr);
            int order   = MIN(MAX_ORDER, MIN(max_fit, align));
            buddy_add_block(addr, order);
            addr += GET_ORDER_SIZE(order);
        }
    }

    LOG_TAGGED_OK("MEMORY", ANSI_BGREEN, "Buddy Allocator Init")
}

uint64_t buddy_allocate(int order, size_t flags) {
    ticketlock_lock(&lock);
    defer ticketlock_unlock(&lock);

    // check if there is already a page in the proper order
    llist_node_t* node = llist_pop(&orders[order]);
    if (node != nullptr) {
        // mark as allocated
        page_t* entry = pfndb_get_entry(FROM_HHDM((node)));
        entry->type = PFNDB_PAGE_ALLOCATED;
        return FROM_HHDM(node);
    }

    // find the next larger order that can be split
    int corder = order + 1;
    while (corder <= MAX_ORDER) {
        node = llist_pop(&orders[corder]);
        if (node) break;
        corder++;
    }

    // no order to split from; allocation failed
    if (!node) {
        if (flags & ALLOC_FLAG_NOFAIL) panic("Buddy: Failed to allocate nofail allocation");
        return 0;
    }

    // split the page down
    uint64_t addr = (uint64_t)node;
    page_t* root_entry = pfndb_get_entry(FROM_HHDM(addr));
    uint8_t max_order = GET_MAX_ORDER(root_entry->order);

    while (corder > order) {
        corder--;
        // update the order of the buddy
        uint64_t buddy = addr + GET_ORDER_SIZE(corder);
        page_t* buddy_entry = pfndb_get_entry(FROM_HHDM((buddy)));
        buddy_entry->type = PFNDB_PAGE_FREE;
        SET_MAX_ORDER(buddy_entry->order, max_order);
        SET_CUR_ORDER(buddy_entry->order, corder);
        // add back to freelist
        llist_push(&orders[corder], (llist_node_t*)buddy);
    }

    // update the order of the main entry
    page_t* main_entry = pfndb_get_entry(FROM_HHDM((addr)));
    assert(main_entry->type == PFNDB_PAGE_FREE);
    main_entry->type = PFNDB_PAGE_ALLOCATED;
    SET_CUR_ORDER(main_entry->order, corder);

    return FROM_HHDM(addr);
}

void buddy_free(uint64_t paddr, int order) {
    ticketlock_lock(&lock);
    defer ticketlock_unlock(&lock);

    uint64_t addr = TO_HHDM(paddr);
    page_t* entry = pfndb_get_entry(paddr);

    int corder = order;
    while (corder < GET_MAX_ORDER(entry->order)) {
        // get the buddy
        uint64_t buddy_paddr = paddr ^ GET_ORDER_SIZE(corder);
        page_t* buddy_entry = pfndb_get_entry(buddy_paddr);

        // only collace if the buddy is free and part of the same order
        if (buddy_entry->type != PFNDB_PAGE_FREE) break;
        if (GET_CUR_ORDER(buddy_entry->order) != corder) break;

        // remove buddy from the freelist
        llist_node_delete(&orders[corder], (llist_node_t*)TO_HHDM(buddy_paddr));

        // get the lower anchor between the two
        if (buddy_paddr < paddr) {
            paddr = buddy_paddr;
            entry = buddy_entry;
            addr = TO_HHDM(paddr);
        }

        // mark the buddy as "allocated" since its part of a larger order now
        buddy_entry->type = PFNDB_PAGE_ALLOCATED;
        corder++;
    }

    entry->type = PFNDB_PAGE_FREE;
    SET_CUR_ORDER(entry->order, corder);
    llist_push(&orders[corder], (llist_node_t*)addr);
}
