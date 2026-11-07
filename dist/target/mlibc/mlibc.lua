local helpers <const> = require("dist.target.helpers")

local pkg = Package {
    name = "mlibc",
    version = "0.0git",
    revision = 1,
    source = require("dist.target.mlibc.source"),
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        helpers.require_target("mlibc-headers"),
        helpers.require_target("libcxx-headers"),
        helpers.require_target("basic-rt"),
        require("dist.target.mlibc.deps").frigg,
        require("dist.target.mlibc.deps").libsmarter,
    },
    runtime_dependencies = {
        helpers.require_target("mlibc-headers"),
        helpers.require_target("basic-rt")
    },
    env = {
        CROSSFILE = helpers.crossFiles[chariot.target_arch]
    },
    configure = [[
        CXXFLAGS="-nostdinc++"                     \
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"   \
        --cross-file="/usr/local/lib/${CROSSFILE}" \
        --prefix="${PREFIX}"                       \
        --buildtype=minsize                        \
        -Ddebug=false                              \
        -Ddefault_library=static                   \
        -Dno_headers=true                          \
        -Db_lto=true
    ]],
    install = [[
        DESTDIR=${INSTALL_DIR} ninja -C ${BUILD_DIR} install
    ]]
}

return pkg
