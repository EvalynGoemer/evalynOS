#pragma once

#include <span>
#include <string_view>
#include <cstdint>

struct TarEntry {
    std::string_view name;
    std::span<const uint8_t> content;
};

class TarIter {
private:
    std::span<const uint8_t> data;
    TarEntry current_entry;

    void finish() {
        data = {};
        current_entry = {};
    }

    void parse();
    void next();

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = TarEntry;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const TarEntry*;
    using reference         = const TarEntry&;

    TarIter() = default;
    explicit TarIter(std::span<const uint8_t> archive) : data(archive) { parse(); }

    TarIter begin() const { return *this; }
    TarIter end() const { return TarIter(); }

    reference operator*() const { return current_entry; }
    pointer operator->() const { return &current_entry; }

    TarIter& operator++() {
        next();
        return *this;
    }

    TarIter operator++(int) {
        TarIter tmp = *this;
        next();
        return tmp;
    }

    friend bool operator==(const TarIter& a, const TarIter& b) {
        return a.data.data() == b.data.data() && a.data.size() == b.data.size();
    }
};
