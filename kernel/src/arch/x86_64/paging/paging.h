#pragma once
#include <arch/generic/panic.h>
#include <arch/generic/paging/paging.h>

#define X86_64_PTE_MASK      0x000ffffffffff000

#define X86_64_PTE_PRESENT   (1ull << 0)
#define X86_64_PTE_WRITABLE  (1ull << 1)
#define X86_64_PTE_USER      (1ull << 2)
#define X86_64_PTE_PWT       (1ull << 3)
#define X86_64_PTE_PCD       (1ull << 4)
#define X86_64_PTE_ACCESSED  (1ull << 5)
#define X86_64_PTE_DIRTY     (1ull << 6)
#define X86_64_PTE_PS        (1ull << 7)  // only on PML2/3 entries
#define X86_64_PTE_PAT       (1ull << 7)  // only on PML1 entries
#define X86_64_PTE_GLOBAL    (1ull << 8)
#define X86_64_PTE_NX        (1ull << 63)

#define X86_64_PTE_UC (X86_64_PTE_PWT | X86_64_PTE_PCD)
#define X86_64_PTE_WC (X86_64_PTE_PAT | X86_64_PTE_PWT)
