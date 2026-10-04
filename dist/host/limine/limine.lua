local LIMINE_VERSION <const> = "12.9.0"

Tool {
    name = "limine",
    version = LIMINE_VERSION,
    revision = 1,
    source = Source {
        name = "limine",
        Archive (
            string.gsub("https://github.com/Limine-Bootloader/Limine/releases/download/v${version}/limine-${version}.tar.xz", "${version}", LIMINE_VERSION),
            "86107e8754365124b1871479f766697461f16ce9d847b3c23bb269335588c565"
        )
    },
    dependencies = {
        "base-devel", "clang", "llvm", "lld", "mtools", "nasm",
    },
    configure = [[
        LDFLAGS="-static" CFLAGS_FOR_TARGET="-O2 -pipe" "${SOURCE_DIR}"/configure --enable-all --prefix="${PREFIX}"
    ]],
    build = [[
        make -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install
        find "${INSTALL_DIR}/${PREFIX}/bin" -maxdepth 1 -type f -exec strip {} + || true
    ]]
}
