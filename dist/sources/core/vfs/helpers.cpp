#include "vfs/helpers.hpp"
#include <memory>
#include <ranges>
#include <tuple>

#include <vfs/vfs.hpp>

namespace vfs {
// returns [parnet, child]
std::tuple<vNodePtrExpect, vNodePtrExpect> resolve_path(std::string_view path, vNodePtr root) {
    // split the path on slashes
    auto segments = path | vfs::RemoveTrailingSlashes | std::views::split('/');
    int total_segments = std::ranges::distance(segments);

    // start at the root
    std::shared_ptr<vNode> current = root;
    std::string_view partname;

    // traverse to the parent of the file
    for (auto [index, part] : segments | std::views::enumerate) {
        if (part.empty()) continue;

        partname = {part.data(), part.size()};
        if (partname == ".") continue;

        // this is the last part of the path
        if (index == total_segments - 1)
            break;

        vfs::vNodePtrExpect next;

        if (partname == "..") {
            next = current->parent;
        } else {
            next = current->get_child(partname);
        }

        // could not resolve the next vnode; propagate error
        if (!next.has_value())
            return {std::unexpected(next.error()), std::unexpected(VfsError::None)};

        current = next.value();
    }

    return {current, current->get_child(partname)};
}

}
