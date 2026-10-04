local NASM_VERSION <const> = "3.02"

Tool {
    name = "nasm",
    version = NASM_VERSION,
    revision = 1,
    source = Source {
        name = "nasm",
        Archive (
            string.gsub("https://www.nasm.us/pub/nasm/releasebuilds/${version}/nasm-${version}.tar.xz", "${version}", NASM_VERSION),
            "87336eba53b4acfe917424ab5d500d2b0054d9f5148d35c2273ccf2cfb712f0d"
        )
    },
    dependencies = {
        "base-devel",
    },
    configure = [[
        LDFLAGS="-static" "${SOURCE_DIR}"/configure --prefix="${PREFIX}"
    ]],
    build = [[
        make -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install
        find "${INSTALL_DIR}/${PREFIX}/bin" -maxdepth 1 -type f -exec strip {} + || true
    ]]
}
