local helpers <const> = require("dist.target.helpers")
local crossfiles <const> = require("dist.sources.crossfiles.crossfiles")

local frigg = Package {
    name = "frigg",
    version = "0.0git",
    revision = 1,
    source = Source {
        name = "frigg",
        Git ("https://github.com/managarm/frigg.git", "b9ce1199489f25add06d66c5cc355e2c17dbe027")
    },
    dependencies = {
        "base-devel", "meson",
        helpers.targetTools,
        crossfiles
    },
    configure = string.format([[
        meson setup ${BUILD_DIR} "${SOURCE_DIR}"     \
        --cross-file="${SOURCES_DIR}/crossfiles/%s"  \
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
        helpers.targetTools,
        crossfiles
    },
    configure = helpers.genericMeson["configure"],
    build = helpers.genericMeson["build"],
    install = helpers.genericMeson["install"],
}

local c_hdrs = Package {
    name = "freestnd-c-hdrs",
    version = "0.0git",
    revision = 1,
    source = Source {
        name = "freestnd-c-hdrs",
        Git ("https://github.com/osdev0/freestnd-c-hdrs.git", "d33711241b46ecb8f2ad33927fcefdcb3ac0162e")
    },
    dependencies = { "base-devel" },
    install = (string.gsub([[
        install -d "${INSTALL_DIR}${PREFIX}/include/freestnd-c-hdrs" "${INSTALL_DIR}${PREFIX}/lib/pkgconfig"
        cp -r "${SOURCE_DIR}/${ARCH}/include/." "${INSTALL_DIR}${PREFIX}/include/freestnd-c-hdrs/"

        pc="${INSTALL_DIR}${PREFIX}/lib/pkgconfig/freestnd-c-hdrs-${ARCH}.pc"
        echo "prefix=${PREFIX}"                                    >  "${pc}"
        echo 'includedir=${prefix}/include'                        >> "${pc}"
        echo "Name: freestnd-c-hdrs-${ARCH}"                       >> "${pc}"
        echo "Description: freestanding headers for mlibc"         >> "${pc}"
        echo "Version: 0"                                          >> "${pc}"
        echo 'Cflags: -idirafter${includedir}/freestnd-c-hdrs'     >> "${pc}"
    ]], "${ARCH}", chariot.target_arch)),
}

local cxx_hdrs = Package {
    name = "freestnd-cxx-hdrs",
    version = "0.0git",
    revision = 1,
    source = Source {
        name = "freestnd-cxx-hdrs",
        Git ("https://github.com/osdev0/freestnd-cxx-hdrs.git", "a6b351e0ab3e74e5789b01fa1447e4cd62373da7")
    },
    dependencies = { "base-devel" },
    install = (string.gsub([[
        install -d "${INSTALL_DIR}${PREFIX}/include/freestnd-cxx-hdrs" "${INSTALL_DIR}${PREFIX}/lib/pkgconfig"
        cp -r "${SOURCE_DIR}/${ARCH}/include/." "${INSTALL_DIR}${PREFIX}/include/freestnd-cxx-hdrs/"

        pc="${INSTALL_DIR}${PREFIX}/lib/pkgconfig/freestnd-cxx-hdrs-${ARCH}.pc"
        echo "prefix=${PREFIX}"                                      >  "${pc}"
        echo 'includedir=${prefix}/include'                          >> "${pc}"
        echo "Name: freestnd-cxx-hdrs-${ARCH}"                       >> "${pc}"
        echo "Description: freestanding headers for mlibc"           >> "${pc}"
        echo "Version: 0"                                            >> "${pc}"
        echo 'Cflags: -isystem${includedir}/freestnd-cxx-hdrs'       >> "${pc}"
    ]], "${ARCH}", chariot.target_arch)),
}

return {
    frigg = frigg,
    libsmarter = libsmarter,
    c_hdrs = c_hdrs,
    cxx_hdrs = cxx_hdrs
}
