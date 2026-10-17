local helpers <const> = require("dist.target.helpers")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

local pkg = Package {
    name = "mlibc",
    version = "0.0git",
    revision = 1,
    source = require("dist.sources.mlibc.mlibc"),
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools, crossfiles,
        helpers.require_target("mlibc-headers"),
        helpers.require_target("basic-rt"),
        require("dist.target.mlibc.deps").frigg,
        require("dist.target.mlibc.deps").libsmarter,
        require("dist.target.mlibc.deps").c_hdrs,
        require("dist.target.mlibc.deps").cxx_hdrs,
    },
    runtime_dependencies = {
        helpers.require_target("mlibc-headers"),
        helpers.require_target("basic-rt")
    },
    configure = string.format([[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"    \
        --cross-file="${SOURCES_DIR}/crossfiles/%s" \
        --prefix="${PREFIX}"                        \
        --buildtype=minsize                         \
        -Ddebug=false                               \
        -Ddefault_library=static                    \
        -Dno_headers=true
    ]], helpers.crossFiles[chariot.target_arch]),
    install = [[
        DESTDIR=${INSTALL_DIR} ninja -C ${BUILD_DIR} install
    ]]
}

return pkg
