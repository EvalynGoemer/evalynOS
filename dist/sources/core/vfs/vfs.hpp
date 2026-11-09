#pragma once

#include <cstddef>
#include <expected>
#include <optional>
#include <memory>
#include <span>
#include <string_view>

using std::size_t;
using std::uint8_t;

namespace vfs {

enum class VfsError {
    None = 0,
    AlreadyExists,
    Overflow,
    NotSupported,
    NotFound
};

struct fileDesc;
class vNode;

extern void init();
extern std::expected<std::shared_ptr<fileDesc>, VfsError> open(std::string_view path, size_t flags);
extern std::expected<std::shared_ptr<fileDesc>, VfsError> mkdir(std::string_view path, size_t flags);

class fileSystem {
public:
    std::shared_ptr<vNode> rootdir;
    virtual VfsError close(fileDesc& fd);

    virtual std::expected<std::shared_ptr<vNode>, VfsError> create_file_vnode();
    virtual std::expected<std::shared_ptr<vNode>, VfsError> create_dir_vnode();

    virtual ~fileSystem() {};
};

extern std::shared_ptr<fileSystem> rootfs;

class vNode {
public:
    std::shared_ptr<vNode> parent;

    virtual std::expected<size_t, VfsError> read(std::span<uint8_t> dest, size_t offset, const fileDesc& fd);
    virtual std::expected<size_t, VfsError> write(std::span<const uint8_t> src, size_t offset, const fileDesc& fd);

    virtual VfsError add_child(std::shared_ptr<vNode> child, std::string_view name);
    virtual std::expected<std::shared_ptr<vNode>, VfsError> get_child(std::string_view name);

    virtual bool isTty();
    virtual ~vNode() {};

};

struct fileDesc {
    std::shared_ptr<vNode> vnode;

    size_t offset;
    size_t flags;

    fileDesc(std::shared_ptr<vNode> vnode, size_t flags) {
        this->vnode = vnode;
        this->flags = flags;
        this->offset = 0;
    }

    std::expected<size_t, VfsError> read(std::span<uint8_t> dest, std::optional<size_t> offset = std::nullopt) {
        return this->vnode->read(dest, offset.value_or(this->offset), *this);
    }

    std::expected<size_t, VfsError> write(std::span<const uint8_t> src, std::optional<size_t> offset = std::nullopt) {
        return this->vnode->write(src, offset.value_or(this->offset), *this);
    }

    template<typename T>
    std::expected<size_t, VfsError> read(T& obj, std::optional<size_t> offset = std::nullopt) {
        if constexpr (std::ranges::range<T>)
            return this->read({reinterpret_cast<uint8_t*>(obj.data()), obj.size() * sizeof(std::ranges::range_value_t<T>)}, offset.value_or(this->offset));
        return this->read({reinterpret_cast<uint8_t*>(&obj), sizeof(obj)}, offset.value_or(this->offset));
    }

    template<typename T>
    std::expected<size_t, VfsError> write(const T& obj, std::optional<size_t> offset = std::nullopt) {
        if constexpr (std::ranges::range<T>)
            return this->write({reinterpret_cast<const uint8_t*>(obj.data()), obj.size() * sizeof(std::ranges::range_value_t<T>)}, offset.value_or(this->offset));
        return this->write({reinterpret_cast<const uint8_t*>(&obj), sizeof(obj)}, offset.value_or(this->offset));
    }

    bool isTty() {
        return this->vnode->isTty();
    }
};

using vNodePtr = std::shared_ptr<vNode>;
using vNodePtrExpect = std::expected<vNodePtr, VfsError>;

}
