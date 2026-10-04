local PKGCONF_VERSION <const> = "3.0.7"

local pkgconf = Tool {
    name = "pkgconf",
    version = PKGCONF_VERSION,
    revision = 1,
    dependencies = {
        "base-devel",
        _ = Source {
            Archive (
                string.gsub("https://github.com/pkgconf/pkgconf/releases/download/pkgconf-${version}/pkgconf-${version}.tar.xz", "${version}", PKGCONF_VERSION),
                     "c926ff491cbd9a331a589160811bd97ab1749b4d5198a519338f2cdfabe6940a"
            )
        }
    },
    configure = [[
        ${SOURCE_DIR}/configure --prefix=${PREFIX}
    ]],
    build = [[
        make -j"${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" make install-strip
        install -d ${INSTALL_DIR}/${PREFIX}/share/pkgconfig/personality.d

        for ARCH in x86_64 riscv64 loongarch64; do
            PERSONALITY_FILE="${INSTALL_DIR}/${PREFIX}/share/pkgconfig/personality.d/${ARCH}-unknown-evalynos.personality"
            echo "Triplet: ${ARCH}-unknown-evalynos" >> "${PERSONALITY_FILE}"
            echo "SysrootDir: ${SYSROOT_DIR}" >> "${PERSONALITY_FILE}"
            echo "DefaultSearchPaths: ${SYSROOT_DIR}/usr/lib/pkgconfig:${SYSROOT_DIR}/usr/share/pkgconfig" >> "${PERSONALITY_FILE}"
            echo "SystemIncludePaths: ${SYSROOT_DIR}/usr/include" >> "${PERSONALITY_FILE}"
            echo "SystemLibraryPaths: ${SYSROOT_DIR}/usr/lib" >> "${PERSONALITY_FILE}"
            ln -sf pkgconf "${INSTALL_DIR}/${PREFIX}/bin/${ARCH}-unknown-evalynos-pkg-config"
            ln -sf pkgconf "${INSTALL_DIR}/${PREFIX}/bin/${ARCH}-unknown-evalynos-pkgconf"
        done
    ]]
}

return pkgconf
