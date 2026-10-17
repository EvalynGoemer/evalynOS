local function require_host(package)
    return require(string.gsub("dist.host.${pkg}.${pkg}", "${pkg}", package))
end

local function require_target(package)
    return require(string.gsub("dist.target.${pkg}.${pkg}", "${pkg}", package))
end

local crossFiles <const> = {
    x86_64      = "evalynos-x86_64.cross",
    riscv64     = "evalynos-riscv64.cross",
    loongarch64 = "evalynos-loongarch64.cross",
}

local targetTriple <const> = {
    x86_64      = "x86_64-unknown-evalynos",
    riscv64     = "riscv64-unknown-evalynos",
    loongarch64 = "loongarch64-unknown-evalynos",
}

local genericMeson = {
    configure = string.format([[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"    \
        --cross-file="${SOURCES_DIR}/crossfiles/%s" \
        --prefix="${PREFIX}"
    ]], crossFiles[chariot.target_arch]),
    build = [[
        meson compile -C ${BUILD_DIR} -j "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" meson install -C ${BUILD_DIR} --strip
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
    crossFiles = crossFiles,
    targetTriple = targetTriple,
    targetTools = targetTools,
    genericMeson = genericMeson,
    require_host = require_host,
    require_target = require_target,
}
