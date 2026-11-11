#pragma once
#include <stdint.h>

static_assert(__atomic_always_lock_free(sizeof(uint64_t), 0));
static_assert(__atomic_always_lock_free(sizeof(uint32_t), 0));

typedef enum {
    AMO_RELAXED = __ATOMIC_RELAXED,
    AMO_ACQUIRE = __ATOMIC_ACQUIRE,
    AMO_RELEASE = __ATOMIC_RELEASE,
    AMO_ACQ_REL = __ATOMIC_ACQ_REL,
    AMO_SEQ_CST = __ATOMIC_SEQ_CST,
} atomic_memorder_t;

// atomic load
static inline uint64_t atomic_load64(uint64_t* ptr, atomic_memorder_t memorder) {
    return __atomic_load_n(ptr, memorder);
}
static inline uint32_t atomic_load32(uint32_t* ptr, atomic_memorder_t memorder) {
    return __atomic_load_n(ptr, memorder);
}
static inline void atomic_ptr_load64(uint64_t* ptr, uint64_t* ret, atomic_memorder_t memorder) {
    __atomic_load(ptr, ret, memorder);
}
static inline void atomic_ptr_load32(uint32_t* ptr, uint32_t* ret, atomic_memorder_t memorder) {
    __atomic_load(ptr, ret, memorder);
}

// atomic store
static inline void atomic_store64(uint64_t* ptr, uint64_t val, atomic_memorder_t memorder) {
    __atomic_store_n(ptr, val, memorder);
}
static inline void atomic_store32(uint32_t* ptr, uint32_t val, atomic_memorder_t memorder) {
    __atomic_store_n(ptr, val, memorder);
}
static inline void atomic_ptr_store64(uint64_t* ptr, uint64_t* val, atomic_memorder_t memorder) {
    __atomic_store(ptr, val, memorder);
}
static inline void atomic_ptr_store32(uint32_t* ptr, uint32_t* val, atomic_memorder_t memorder) {
    __atomic_store(ptr, val, memorder);
}

// atomic add
static inline uint64_t atomic_fetch_add64(uint64_t* ptr, uint64_t val, atomic_memorder_t memorder) {
    return __atomic_fetch_add(ptr, val, memorder);
}
static inline uint32_t atomic_fetch_add32(uint32_t* ptr, uint32_t val, atomic_memorder_t memorder) {
    return __atomic_fetch_add(ptr, val, memorder);
}
static inline uint64_t atomic_add_fetch64(uint64_t* ptr, uint64_t val, atomic_memorder_t memorder) {
    return __atomic_add_fetch(ptr, val, memorder);
}
static inline uint32_t atomic_add_fetch32(uint32_t* ptr, uint32_t val, atomic_memorder_t memorder) {
    return __atomic_add_fetch(ptr, val, memorder);
}

// atomic sub
static inline uint64_t atomic_fetch_sub64(uint64_t* ptr, uint64_t val, atomic_memorder_t memorder) {
    return __atomic_fetch_sub(ptr, val, memorder);
}
static inline uint32_t atomic_fetch_sub32(uint32_t* ptr, uint32_t val, atomic_memorder_t memorder) {
    return __atomic_fetch_sub(ptr, val, memorder);
}
static inline uint64_t atomic_sub_fetch64(uint64_t* ptr, uint64_t val, atomic_memorder_t memorder) {
    return __atomic_sub_fetch(ptr, val, memorder);
}
static inline uint32_t atomic_sub_fetch32(uint32_t* ptr, uint32_t val, atomic_memorder_t memorder) {
    return __atomic_sub_fetch(ptr, val, memorder);
}

// atomic cas
static inline bool atomic_cas64(uint64_t* ptr, uint64_t* expected, uint64_t desired, atomic_memorder_t success_memorder, atomic_memorder_t fail_memorder) {
    return __atomic_compare_exchange_n(ptr, expected, desired, false, success_memorder, fail_memorder);
}
static inline bool atomic_cas64_weak(uint64_t* ptr, uint64_t* expected, uint64_t desired, atomic_memorder_t success_memorder, atomic_memorder_t fail_memorder) {
    return __atomic_compare_exchange_n(ptr, expected, desired, true, success_memorder, fail_memorder);
}
static inline bool atomic_cas32(uint32_t* ptr, uint32_t* expected, uint32_t desired, atomic_memorder_t success_memorder, atomic_memorder_t fail_memorder) {
    return __atomic_compare_exchange_n(ptr, expected, desired, false, success_memorder, fail_memorder);
}
static inline bool atomic_cas32_weak(uint32_t* ptr, uint32_t* expected, uint32_t desired, atomic_memorder_t success_memorder, atomic_memorder_t fail_memorder) {
    return __atomic_compare_exchange_n(ptr, expected, desired, true, success_memorder, fail_memorder);
}

// atomic exchange
static inline uint64_t atomic_xchg64(uint64_t* ptr, uint64_t val, atomic_memorder_t memorder) {
    return __atomic_exchange_n(ptr, val, memorder);
}
static inline uint32_t atomic_xchg32(uint32_t* ptr, uint32_t val, atomic_memorder_t memorder) {
    return __atomic_exchange_n(ptr, val, memorder);
}
