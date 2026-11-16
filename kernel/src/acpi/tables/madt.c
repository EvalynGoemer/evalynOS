#include <stdint.h>
#include <stddef.h>
#include <acpi/tables/madt.h>

struct MADTEntryHeader* madt_first_entry(struct MADT* madt) {
    uint8_t* base = (uint8_t*)madt;
    uint8_t* end  = base + madt->header.length;
    uint8_t* ptr  = base + sizeof(struct MADT);

    if (ptr + sizeof(struct MADTEntryHeader) > end)
        return NULL;

    struct MADTEntryHeader* entry = (void*)ptr;

    if (entry->length < sizeof(struct MADTEntryHeader))
        return NULL;

    if (ptr + entry->length > end)
        return NULL;

    return entry;
}

struct MADTEntryHeader* madt_next_entry(struct MADT* madt, struct MADTEntryHeader* entry) {
    if (entry == NULL)
        return madt_first_entry(madt);

    uint8_t* base = (uint8_t*)madt;
    uint8_t* end  = base + madt->header.length;
    uint8_t* ptr  = (uint8_t*)entry + entry->length;

    if (ptr + sizeof(struct MADTEntryHeader) > end)
        return NULL;

    struct MADTEntryHeader* next = (struct MADTEntryHeader*)ptr;

    if (next->length < sizeof(struct MADTEntryHeader))
        return NULL;

    if (ptr + next->length > end)
        return NULL;

    return next;
}
