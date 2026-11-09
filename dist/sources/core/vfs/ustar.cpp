#include <klib/misc.hpp>
#include <vfs/ustar.hpp>
#include <charconv>

constexpr unsigned int USTAR_BLOCK_SIZE  = 512;
constexpr unsigned int USTAR_NAME_OFFSET = 0;
constexpr unsigned int USTAR_NAME_LEN    = 100;
constexpr unsigned int USTAR_SIZE_OFFSET = 124;
constexpr unsigned int USTAR_SIZE_LEN    = 12;

void TarIter::parse() {
    if (data.size() < USTAR_BLOCK_SIZE) return finish();
    if (data[USTAR_NAME_OFFSET] == '\0') return finish();

    std::string_view name_field(reinterpret_cast<const char*>(data.data()) + USTAR_NAME_OFFSET, USTAR_NAME_LEN);
    std::string_view size_field(reinterpret_cast<const char*>(data.data()) + USTAR_SIZE_OFFSET, USTAR_SIZE_LEN);

    size_t size = 0;
    std::from_chars(size_field.begin(), size_field.end(), size, 8);

    if (size > data.size() - USTAR_BLOCK_SIZE)
        return finish();

    current_entry.name = name_field.substr(0, name_field.find('\0'));
    current_entry.content = data.subspan(USTAR_BLOCK_SIZE, size);
}

void TarIter::next() {
    size_t step = USTAR_BLOCK_SIZE + klib::align_up(current_entry.content.size(), USTAR_BLOCK_SIZE);
    if (step >= data.size()) return finish();
    data = data.subspan(step);
    parse();
}
