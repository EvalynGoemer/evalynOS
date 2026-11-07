local helpers <const> = require("dist.target.helpers")
local llvm <const> = require("dist.host.llvm.llvm")

local pkg = Package {
    name = "libcxx-headers",
    version = "0.0git",
    revision = 1,
    source = llvm.source,
    dependencies = {
        "base-devel", "cmake", "ninja",
        helpers.targetTools,
        helpers.require_target("mlibc-headers"),
    },
    env = {
        TARGET_TRIPLE = helpers.targetTriple[chariot.target_arch]
    },
    configure = [[
        cmake -S "${SOURCE_DIR}/runtimes" -B "${BUILD_DIR}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release                                   \
        -DLLVM_ENABLE_LTO=Full                                       \
        -DCMAKE_INSTALL_PREFIX="${PREFIX}"                           \
        -DCMAKE_SYSROOT="${SYSROOT_DIR}"                             \
        -DCMAKE_C_COMPILER_TARGET="${TARGET_TRIPLE}"                 \
        -DCMAKE_CXX_COMPILER_TARGET="${TARGET_TRIPLE}"               \
        -DCMAKE_ASM_COMPILER_TARGET="${TARGET_TRIPLE}"               \
        -DLLVM_DEFAULT_TARGET_TRIPLE="${TARGET_TRIPLE}"              \
        -DCMAKE_C_COMPILER=clang                                     \
        -DCMAKE_CXX_COMPILER=clang++                                 \
        -DCMAKE_ASM_COMPILER=clang                                   \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY               \
        -DLIBUNWIND_ENABLE_SHARED=OFF                                \
        -DLIBCXX_ENABLE_SHARED=OFF                                   \
        -DLIBCXX_ENABLE_THREADS=ON                                   \
        -DLIBCXX_HAS_PTHREAD_API=ON                                  \
        -DLIBCXXABI_ENABLE_SHARED=OFF                                \
        -DLIBCXXABI_ENABLE_THREADS=ON                                \
        -DLLVM_ENABLE_RUNTIMES='libunwind;libcxxabi;libcxx'          \
        -DLLVM_INCLUDE_TESTS=OFF
    ]],
    build = [[
        cmake --build ${BUILD_DIR} --target generate-cxx-headers generate-cxxabi-headers --parallel "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" cmake --build ${BUILD_DIR} --target install-cxx-headers install-cxxabi-headers
    ]],
}

return pkg
