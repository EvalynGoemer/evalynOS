local helpers <const> = require("dist.target.helpers")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

local klib = Package {
    name = "klib",
    version = "0.0git",
    revision = 1,
    dependencies = {
        "base-devel", "clang", "llvm", "lld", "meson",
        crossfiles = crossfiles,
        _ = Source { Local("dist/sources/klib")}
    },
    configure = string.format([[
        meson setup build "${SOURCE_DIR}"           \
        --cross-file="${SOURCES_DIR}/crossfiles/%s" \
        --prefix="${PREFIX}"
    ]], helpers.getCrossFile()),
    build = [[
        meson compile -C build -j "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" meson install -C build --strip
    ]]
}

return klib
