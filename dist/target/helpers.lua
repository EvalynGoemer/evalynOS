-- internal

local cross_files <const> = {
    x86_64      = "evalynos-x86_64.cross",
    riscv64     = "evalynos-riscv64.cross",
    loongarch64 = "evalynos-loongarch64.cross",
}

local function require_host(package)
    return require(string.gsub("dist.host.${pkg}.${pkg}", "${pkg}", package))
end

-- exported

local genericMeson = {
    configure = string.format([[
        meson setup build "${SOURCE_DIR}"           \
        --cross-file="${SOURCES_DIR}/crossfiles/%s" \
        --prefix="${PREFIX}"
    ]], cross_files[chariot.target_arch]),
    build = [[
        meson compile -C build -j "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" meson install -C build --strip
    ]]
}

local targetTools = Tool {
    name = "targetTools",
    version = "0.0meta",
    revision = 1,
    runtime_dependencies = {
        require_host("llvm").package,
        require_host("pkgconf"),
    },
    install = "exit 0"
}

return {
    genericMeson = genericMeson,
    targetTools = targetTools,
}
