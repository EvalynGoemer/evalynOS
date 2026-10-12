#pragma once

#ifndef __ASSEMBLER__
#include <utils/limine.h>
#endif

// C/GAS macros
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

// C only macros
#ifndef __ASSEMBLER__
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_UP(x, align)   ((((uint64_t)(x)) + ((align) - 1)) & ~((uint64_t) ((align) - 1)))
#define ALIGN_DOWN(x, align) (((uint64_t)(x)) & ~((uint64_t) ((align) - 1)))
#define IS_ALIGNED(x, align) (((uint64_t)(x) & ((align) - 1)) == 0)
#define MISALIGNMENT(x, align) (((uint64_t)(x) & ((align) - 1)))

#define TO_HHDM(x)     ((uintptr_t)(x) + hhdm_offset)
#define FROM_HHDM(x)   ((uintptr_t)(x) - hhdm_offset)
#define TO_HHDM_PTR(x) ((void*)TO_HHDM(x))

#define LIKELY(x)     __builtin_expect(!!(x), 1)
#define UNLIKELY(x)   __builtin_expect(!!(x), 0)
#define UNREACHABLE() __builtin_unreachable()

#define UNUSED(x)    ((void)(x))
#define MAYBE_UNUSED [[gnu::unused]]

#define CONTAINER_OF(PTR, TYPE, MEMBER)                                                                                               \
    ({                                                                                                                                \
        static_assert(__builtin_types_compatible_p(typeof(((TYPE*) 0)->MEMBER), typeof(*PTR)), "member type does not match pointer"); \
        (TYPE*) (((uintptr_t) (PTR)) - __builtin_offsetof(TYPE, MEMBER));                                                             \
    })

#endif

// GAS only macros
#ifdef __ASSEMBLER__
    #define ZERO_TO_31 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31

    #define ITER_REGS(REGS, ...)    ;\
        .macro CAT(iter_, __LINE__) ;\
            .irp i, REGS            ;\
                __VA_ARGS__         ;\
            .endr                   ;\
        .endm                       ;\
        CAT(iter_, __LINE__)
#endif
