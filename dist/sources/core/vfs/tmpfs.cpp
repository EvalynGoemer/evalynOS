#include <memory>

#include <vfs/vfs.hpp>
#include <vfs/helpers.hpp>
#include <vfs/tmpfs.hpp>

#include <abi-bits/fcntl.h>

namespace vfs {

tmpfs::tmpfs() {
    this->rootdir = std::make_shared<tmpfsDirVnode>();
}

// nop for tmpfs
VfsError tmpfs::close(fileDesc&) {
    return VfsError::None;
}

std::expected<size_t, VfsError> tmpfsFileVnode::read(std::span<uint8_t> dest, size_t offset, const fileDesc&) {
    auto amount = dest.size();
    auto size = this->data.size();

    if (offset >= size)
        return 0;

    if ((amount + offset) > size)
        amount = size - offset;

    std::copy_n(this->data.begin() + offset, amount, dest.begin());
    return amount;
}

std::expected<size_t, VfsError> tmpfsFileVnode::write(std::span<const uint8_t> src, size_t offset, const fileDesc&) {
    auto amount = src.size();
    auto size = this->data.size();

    if (amount > (std::numeric_limits<size_t>::max() - offset))
        return std::unexpected(VfsError::Overflow);

    if ((amount + offset) > size)
        this->data.resize(amount + offset, 0);

    std::copy(src.begin(), src.end(), this->data.begin() + offset);

    return amount;
}

std::expected<std::shared_ptr<vNode>, VfsError> tmpfsDirVnode::get_child(std::string_view name) {
    auto child = this->dirents.find(std::string(name));
    if (child == this->dirents.end())
        return std::unexpected(VfsError::NotFound);
    return child->second;
}

VfsError tmpfsDirVnode::add_child(std::shared_ptr<vNode> child, std::string_view name) {
    this->dirents.emplace(name, child);
    return VfsError::None;
}

std::expected<std::shared_ptr<vNode>, VfsError> tmpfs::create_file_vnode() {
    return std::make_shared<tmpfsFileVnode>();
}
std::expected<std::shared_ptr<vNode>, VfsError> tmpfs::create_dir_vnode() {
    return std::make_shared<tmpfsDirVnode>();
}

}
