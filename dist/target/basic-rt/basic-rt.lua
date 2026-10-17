local helpers <const> = require("dist.target.helpers")
local llvm    <const> = require("dist.host.llvm.llvm")

local pkg = Package {
    name = "basic-rt",
    version = "0.0git",
    revision = 1,
    source = llvm.source,
    dependencies = {
        "base-devel", "cmake", "ninja",
        helpers.require_target("mlibc-headers"),
        helpers.targetTools,
    },
    configure = (string.gsub([[
        cmake -S "${SOURCE_DIR}/compiler-rt" -B "${BUILD_DIR}" -G Ninja  \
        -DCMAKE_BUILD_TYPE=MinSizeRel                                    \
        -DCMAKE_INSTALL_PREFIX="${PREFIX}"                               \
        -DCMAKE_SYSROOT="${SYSROOT_DIR}"                                 \
        -DCMAKE_C_COMPILER_TARGET="${TARGET_TRIPLE}"                     \
        -DCMAKE_CXX_COMPILER_TARGET="${TARGET_TRIPLE}"                   \
        -DCMAKE_ASM_COMPILER_TARGET="${TARGET_TRIPLE}"                   \
        -DCMAKE_C_COMPILER=clang                                         \
        -DCMAKE_CXX_COMPILER=clang++                                     \
        -DCMAKE_ASM_COMPILER=clang                                       \
        -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON                             \
        -DCOMPILER_RT_BUILD_BUILTINS=ON                                  \
        -DCOMPILER_RT_BUILD_CRT=ON                                       \
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF                                \
        -DCOMPILER_RT_BUILD_MEMPROF=OFF                                  \
        -DCOMPILER_RT_BUILD_PROFILE=OFF                                  \
        -DCOMPILER_RT_BUILD_CTX_PROFILE=OFF                              \
        -DCOMPILER_RT_BUILD_SANITIZERS=OFF                               \
        -DCOMPILER_RT_BUILD_XRAY=OFF                                     \
        -DCOMPILER_RT_BUILD_ORC=OFF                                      \
        -DCOMPILER_RT_OS_DIR=evalynos                                    \
        -DCOMPILER_RT_INSTALL_PATH:PATH=lib/clang/23                     \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY                   \
        -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON
    ]], "${TARGET_TRIPLE}", helpers.targetTriple[chariot.target_arch])),
    build = [[
        cmake --build ${BUILD_DIR} --parallel "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" cmake --install ${BUILD_DIR}
    ]],
}

return pkg
