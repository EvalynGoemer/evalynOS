#!/usr/bin/env bash

set -euo pipefail

DOWNLOAD=0; [[ "${1-}" == "--download" ]] && DOWNLOAD=1

cd "$(dirname -- "$0")"

PROJECT_DIR="$(realpath ../)"
JINX_DIR="$(realpath ../jinx/)"
KERNEL_DIR="$(realpath ../kernel/)"

clone_repo_commit() {
    if test -d "$2/.git"; then
        git -C "$2" reset --hard
        git -C "$2" clean -fd
        if ! git -C "$2" -c advice.detachedHead=false checkout $3; then
            rm -rf "$2"
        fi
    else
        if test -d "$2"; then
            echo "error: '$2' is not a Git repository" 1>&2
            exit 1
        fi
    fi
    if ! test -d "$2"; then
        git clone $1 "$2"
        if ! git -C "$2" -c advice.detachedHead=false checkout $3; then
            rm -rf "$2"
            exit 1
        fi
    fi
}

clone_repo_commit                                      \
    https://github.com/Mintsuki/Jinx.git               \
    "$JINX_DIR"                                        \
    6ed3ee6ece09b3b2558e5434e7f3991cd12cbd27

cd "$JINX_DIR"
./jinx init ..

host_pkgs=(limine llvm nasm ovmf2-bin qemu xorriso)

cmd=build; [[ ${DOWNLOAD} == 1 ]] && cmd=download
echo "${cmd}ing host packages"
for pkg in "${host_pkgs[@]}"; do
    ./jinx "$cmd" "host:$pkg"
done

for pkg in "${host_pkgs[@]}"; do
    mkdir -p "$JINX_DIR/host-pkgs/$pkg"
    xbps="$(ls "$JINX_DIR/host-pkgs/${pkg}"-*.x86_64.xbps 2>/dev/null | sort -V | tail -n1)"
    tar -xf "$xbps" -C "$JINX_DIR/host-pkgs/$pkg"
done

for arch in riscv64 loongarch64; do
    build_dir="${JINX_DIR}-${arch}"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    ln -s ../jinx/host-pkgs   "${build_dir}/host-pkgs"
    ln -s ../jinx/host-builds "${build_dir}/host-builds"
    (cd "${build_dir}" && "${JINX_DIR}/jinx" init .. ARCH="${arch}")
done

cd "$KERNEL_DIR"
echo "getting kernel deps"
./get-deps

cd "$PROJECT_DIR"
echo "generating initramfs"
make initramfs
echo "generating iso"
make mkiso

echo "finished bootstrap"
