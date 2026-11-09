#pragma once

#include <string_view>

namespace posix {
    bool spawn(std::string_view path);
}
