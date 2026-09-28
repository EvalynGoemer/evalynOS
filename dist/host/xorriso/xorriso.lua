local XORRISO_VERSION <const> = "1.5.8.pl02"

Tool {
    name = "xorriso",
    version = XORRISO_VERSION,
    revision = 1,
    dependencies = {
        "base-devel", "musl", "kernel-headers-musl",
        _ = Source {
            Archive (
                string.gsub("https://mirrors.ocf.berkeley.edu/gnu/xorriso/xorriso-${version}.tar.gz", "${version}", XORRISO_VERSION),
                "b1455ecafbf0692ddafe1d71002a96f2ce2d77f4deae602678261ce033f97bc8"
            )
        }
    },
    configure = [[
        CC="musl-gcc" "${SOURCE_DIR}"/configure --prefix="${PREFIX}" --disable-shared --enable-static
    ]],
    build = [[
        make -j"${PARALLELISM}" LDFLAGS="-all-static"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install
        find "${INSTALL_DIR}/${PREFIX}/bin" -maxdepth 1 -type f -exec strip {} + || true
    ]]
}
