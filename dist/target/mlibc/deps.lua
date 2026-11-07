local helpers <const> = require("dist.target.helpers")

local frigg = Package {
    name = "frigg",
    version = "0.0git",
    revision = 1,
    source = Source {
        name = "frigg",
        Git ("https://github.com/managarm/frigg.git", "509d60cfc0eef8fad690bb9f0f2a90fe621a8181")
    },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools
    },
    configure = string.format([[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"     \
        --cross-file="/usr/local/lib/%s"             \
        --prefix="${PREFIX}"                         \
        -Dbuild_tests=disabled
    ]], helpers.crossFiles[chariot.target_arch]),
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}

local libsmarter = Package {
    name = "libsmarter",
    version = "0.0git",
    revision = 1,
    source = Source {
        name = "libsmarter",
        Git ("https://github.com/managarm/libsmarter.git", "f7d061bc37d485418344452c7ceb28d5df3ba85d")
    },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}

return {
    frigg = frigg,
    libsmarter = libsmarter,
}
