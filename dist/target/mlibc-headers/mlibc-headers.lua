local helpers <const> = require("dist.target.helpers")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

local pkg = Package {
    name = "mlibc-headers",
    version = "0.0git",
    revision = 1,
    source = require("dist.sources.mlibc.mlibc"),
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        crossfiles
    },
    configure = string.format([[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"    \
        --cross-file="${SOURCES_DIR}/crossfiles/%s" \
        --prefix="${PREFIX}"                        \
        -Dheaders_only=true
    ]], helpers.crossFiles[chariot.target_arch]),
    install = [[
        DESTDIR=${INSTALL_DIR} ninja -C ${BUILD_DIR} install
    ]]
}

return pkg
