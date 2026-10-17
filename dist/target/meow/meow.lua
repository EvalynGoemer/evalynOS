local helpers <const> = require("dist.target.helpers")
local mlibc <const> = require("dist.target.mlibc.mlibc")

Package {
    name = "meow",
    version = "0.0git",
    revision = 1,
    source = Source {
        name = "meow",
        Git ("https://github.com/robotman2412/meow.git", "78a9b7e9fe09ba44837dc461b1bf81fbe6299e22")
    },
    dependencies = {
        "base-devel",
        helpers.targetTools, mlibc,
    },
    build = string.format([[
        cd "${BUILD_DIR}"
        clang --target=%s --sysroot="${SYSROOT_DIR}" -O2 -static "${SOURCE_DIR}/meow.c" -o meow
    ]], helpers.targetTriple[chariot.target_arch]),
    install = [[
        install -Dm755 "${BUILD_DIR}/meow" "${INSTALL_DIR}${PREFIX}/bin/meow.elf"
    ]],
}
