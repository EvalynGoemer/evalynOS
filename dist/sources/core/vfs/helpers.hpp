#pragma once

#include <vfs/vfs.hpp>

namespace vfs {

extern std::tuple<vNodePtrExpect, vNodePtrExpect> resolve_path(std::string_view path, vNodePtr root);

template <typename T>
requires std::invocable<T, std::string_view>
auto operator|(std::string_view sv, T func) {
    return func(sv);
}

static inline std::string_view RemoveTrailingSlashes(std::string_view sv) {
    auto last_not_slash = sv.find_last_not_of('/');

    // handle strings of only slashes and empty strings
    if (last_not_slash == std::string_view::npos)
        return sv.empty() ? sv : sv.substr(0, 1);

    return sv.substr(0, last_not_slash + 1);
}

static inline std::string_view LastPathComponent(std::string_view sv) {
    std::string_view clean = sv | RemoveTrailingSlashes;
    auto pos = clean.rfind('/');

    if (pos == std::string_view::npos)
        return clean;

    return clean.substr(pos + 1);
}

}
