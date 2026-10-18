local helpers <const> = require("dist.target.helpers")
local llvm <const> = require("dist.host.llvm.llvm")

local pkg = Package {
    name = "libcxx",
    version = "0.0git",
    revision = 1,
    source = llvm.source,
    dependencies = {
        "base-devel", "cmake", "ninja",
        helpers.targetTools,
        helpers.require_target("mlibc-headers"),
        helpers.require_target("basic-rt"),
    },
    runtime_dependencies = {
        helpers.require_target("mlibc"),
    },
    configure = (string.gsub([[
        cmake -S "${SOURCE_DIR}/runtimes" -B "${BUILD_DIR}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release                                   \
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
        -DLIBCXX_ENABLE_THREADS=OFF                                  \
        -DLIBCXXABI_ENABLE_SHARED=OFF                                \
        -DLIBCXXABI_ENABLE_THREADS=OFF                               \
        -DLLVM_ENABLE_RUNTIMES='libunwind;libcxxabi;libcxx'          \
        -DLLVM_INCLUDE_TESTS=OFF
    ]], "${TARGET_TRIPLE}", helpers.targetTriple[chariot.target_arch])),
    build = [[
        cmake --build ${BUILD_DIR} --parallel "${PARALLELISM}"
    ]],
    install = [[
        DESTDIR="${INSTALL_DIR}" cmake --install ${BUILD_DIR}
    ]],
}

return pkg
