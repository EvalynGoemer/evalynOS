#include <klib/misc.hpp>
#include <sys/auxv.h>

namespace klib {
    const uint64_t PAGE_SIZE = getauxval(AT_PAGESZ);
}
