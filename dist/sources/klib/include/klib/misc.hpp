#include <concepts>
#include <cstdint>

namespace klib {
    extern const uint64_t PAGE_SIZE;

    template<std::integral T, std::integral U>
    static inline constexpr T align_up(T x, U align) {
        return (x + (align - 1)) & ~(align - 1);
    }

    template<std::integral T, std::integral U>
    static inline constexpr T align_down(T x, U align) {
        return x & ~(align - 1);
    }
}
