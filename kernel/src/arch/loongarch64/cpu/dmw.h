#pragma once

#define DMW_PLV0 (1ull << 0)
#define DMW_PLV1 (1ull << 1)
#define DMW_PLV2 (1ull << 2)
#define DMW_PLV3 (1ull << 3)

#define DMW_MAT_CACHED           (1 << 4)
#define DMW_MAT_WEAK_UNCACHED    (2 << 4)
#define DMW_MAT_STRONG_UNCACHED  (0 << 4)

#define DMW_VSEG_SHIFT 60

#define HHDM_CACHED_OFFSET           0x8000000000000000ull
#define HHDM_WEAK_UNCACHED_OFFSET    0xA000000000000000ull
#define HHDM_STRONG_UNCACHED_OFFSET  0xB000000000000000ull

extern void setup_dmw();
