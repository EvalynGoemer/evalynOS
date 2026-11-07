local helpers <const> = require("dist.target.helpers")

local pkg = Package {
    name = "mlibc-headers",
    version = "0.0git",
    revision = 1,
    source = require("dist.target.mlibc.source"),
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        require("dist.target.mlibc.deps").frigg,
    },
    env = {
        CROSSFILE = helpers.crossFiles[chariot.target_arch]
    },
    configure = [[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"   \
        --cross-file="/usr/local/lib/${CROSSFILE}" \
        --prefix="${PREFIX}"                       \
        -Dheaders_only=true
    ]],
    install = [[
        DESTDIR=${INSTALL_DIR} ninja -C ${BUILD_DIR} install
    ]]
}

return pkg
