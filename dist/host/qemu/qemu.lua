local QEMU_VERSION <const> = "11.1.1"

Tool {
    name = "qemu",
    version = QEMU_VERSION,
    revision = 1,
    source = Source {
        name = "qemu",
        Archive (
            string.gsub("https://download.qemu.org/qemu-${version}.tar.xz", "${version}", QEMU_VERSION),
            "079ffbff8a7111bbc89022107cbabf3bbfd614d5fc9d7cc675991196aca12482"
        ),
        patches = {"dist/host/qemu/patches/0001-x86-la57-info-mem-fix.patch"}
    },
    dependencies = {
        "base-devel", "meson", "ninja", "dtc", "gtk4", "vte3", "libpulse",
    },
    configure = [[
        cp -a "${SOURCE_DIR}" ./source
        mkdir build
        cd build
        ../source/configure                                                  \
            --prefix="${PREFIX}"                                             \
            --target-list=x86_64-softmmu,riscv64-softmmu,loongarch64-softmmu \
            --disable-docs
    ]],
    build = [[
        cd build
        make -j"${PARALLELISM}"
    ]],
    install = [[
        cd build
        DESTDIR="${INSTALL_DIR}" make install
        find "${INSTALL_DIR}/${PREFIX}/bin" -maxdepth 1 -type f -exec strip {} + || true
    ]]
}
