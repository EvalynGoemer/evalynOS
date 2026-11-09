#include "vfs/vfs.hpp"
#include <iostream>
#include <memory>

#include <klib/initramfs.hpp>
#include <vfs/ustar.hpp>
#include <vfs/tmpfs.hpp>
#include <vfs/helpers.hpp>

#include <abi-bits/fcntl.h>

namespace vfs {
    std::shared_ptr<fileSystem> rootfs;

    std::expected<std::shared_ptr<fileDesc>, VfsError> open(std::string_view path, size_t flags) {
        auto [parent, child] = vfs::resolve_path(path, rootfs->rootdir);

        if (!parent.has_value())
            return std::unexpected(parent.error());

        if (!child.has_value()) {
            auto filename = path | vfs::LastPathComponent;

            // . and .. might not be real vnodes so emulate when missing
            if (child.error() == VfsError::NotFound && filename == ".")
                return std::make_shared<fileDesc>(child.value(), flags);
            if (child.error() == VfsError::NotFound && filename == "..")
                return std::make_shared<fileDesc>(parent.value(), flags);

            // if its missing and it was requested create the file
            if (child.error() == VfsError::NotFound && (flags & O_CREAT)) {
                auto newf = rootfs->create_file_vnode().value();
                parent.value()->add_child(newf, std::string(filename));
                return std::make_shared<fileDesc>(newf, flags);
            }
            // otherwise propagate the error
            return std::unexpected(child.error());
        }

        return std::make_shared<fileDesc>(child.value(), flags);
    }

    std::expected<std::shared_ptr<fileDesc>, VfsError> mkdir(std::string_view path, size_t flags) {
        auto [parent, child] = vfs::resolve_path(path, rootfs->rootdir);

        // if we couldnt get the parent propagate the error
        if (!parent.has_value())
            return std::unexpected(parent.error());

        // if the child had no error it alreay existed
        if (child.has_value())
            return std::unexpected(VfsError::AlreadyExists);

        // "." and ".." might not be real vnodes but they always exist
        auto filename = path | vfs::LastPathComponent;
        if (filename == "." || filename == "..")
            return std::unexpected(VfsError::AlreadyExists);

        auto newf = rootfs->create_dir_vnode().value();
        parent.value()->add_child(newf, std::string(filename));
        return std::make_shared<fileDesc>(newf, flags);
    }

    void init() {
        rootfs = std::make_shared<vfs::tmpfs>();
        auto initramfs = klib::initramfs::get();

        if (!initramfs.has_value()) {
            std::cerr << "[CORE] FATAL ERROR: Could not get initramfs" << std::endl;
            std::exit(-1);
        }

        for (auto& e : TarIter(initramfs.value())) {
            bool is_dir = e.name.back() == '/';
            auto path = e.name.substr(1);

            if (is_dir) {
                // just attempt to make the folder and ignore failure
                vfs::mkdir(path, 0);
            } else {
                auto f = vfs::open(path, O_CREAT);

                // if adding an actual file failed abort
                if (!f.has_value()) {
                    std::cerr << "[CORE] FATAL ERROR: Could not add " << path << " to the tmpfs" << std::endl;
                    std::exit(-1);
                }

                f.value()->write(e.content);
                std::cout << "[CORE] Added " << path << " to the tmpfs" << std::endl;
            }
        }
    }
}
