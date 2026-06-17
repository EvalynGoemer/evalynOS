#pragma once
#include <utils/limine.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_UP(x, align)   ((((uint64_t)(x)) + ((align) - 1)) & ~((uint64_t) ((align) - 1)))
#define ALIGN_DOWN(x, align) (((uint64_t)(x)) & ~((uint64_t) ((align) - 1)))
#define IS_ALIGNED(x, align) (((uint64_t)(x) & ((align) - 1)) == 0)

#define TO_HHDM(x)     ((uintptr_t)(x) + hhdm_request.response->offset)
#define FROM_HHDM(x)   ((uintptr_t)(x) - hhdm_request.response->offset)
#define TO_HHDM_PTR(x) ((void*)TO_HHDM(x))

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define UNUSED(x)   ((void)(x))

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define CONTAINER_OF(PTR, TYPE, MEMBER)                                                                                               \
    ({                                                                                                                                \
        static_assert(__builtin_types_compatible_p(typeof(((TYPE*) 0)->MEMBER), typeof(*PTR)), "member type does not match pointer"); \
        (TYPE*) (((uintptr_t) (PTR)) - __builtin_offsetof(TYPE, MEMBER));                                                             \
    })
