#pragma once

#include <expected>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <span>

#include <vfs/vfs.hpp>

namespace vfs {

class tmpfsDirVnode;

class tmpfs : public fileSystem {
public:
    tmpfs();

    VfsError close(fileDesc& fd) override;

    std::expected<std::shared_ptr<vNode>, VfsError> create_file_vnode() override;
    std::expected<std::shared_ptr<vNode>, VfsError> create_dir_vnode() override;
};

class tmpfsFileVnode : public vNode {
    std::vector<uint8_t> data;
public:
    std::expected<size_t, VfsError> read(std::span<uint8_t> dest, size_t offset, const fileDesc& fd) override;
    std::expected<size_t, VfsError> write(std::span<const uint8_t> src, size_t offset, const fileDesc& fd) override;
};

class tmpfsDirVnode : public vNode {
    std::unordered_map<std::string, std::shared_ptr<vNode>> dirents;
public:
    std::expected<std::shared_ptr<vNode>, VfsError> get_child(std::string_view name) override;
    VfsError add_child(std::shared_ptr<vNode> child, std::string_view name) override;
};

}
