#include <vfs/vfs.hpp>

namespace vfs {

std::expected<size_t, VfsError> vNode::read(std::span<uint8_t>, size_t, const fileDesc&) {
    return std::unexpected(VfsError::NotSupported);
}

std::expected<size_t, VfsError> vNode::write(std::span<const uint8_t>, size_t, const fileDesc&) {
    return std::unexpected(VfsError::NotSupported);
}

VfsError vNode::add_child(std::shared_ptr<vNode>, std::string_view) {
    return VfsError::NotSupported;
}

std::expected<std::shared_ptr<vNode>, VfsError> vNode::get_child(std::string_view) {
    return std::unexpected(VfsError::NotSupported);
}

bool vNode::isTty() {
    return false;
}

VfsError fileSystem::close(fileDesc&) {
    return VfsError::NotSupported;
}

std::expected<std::shared_ptr<vNode>, VfsError> fileSystem::create_file_vnode() {
    return std::unexpected(VfsError::NotSupported);
}
std::expected<std::shared_ptr<vNode>, VfsError> fileSystem::create_dir_vnode() {
    return std::unexpected(VfsError::NotSupported);
}

}
