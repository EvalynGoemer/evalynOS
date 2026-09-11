#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

case "${ARCH}" in
    x86_64)      LIMINE_ARCH="x86-64";      BUILD_DIR="../jinx";;
    riscv64)     LIMINE_ARCH="riscv64";     BUILD_DIR="../jinx-riscv64";;
    loongarch64) LIMINE_ARCH="loongarch64"; BUILD_DIR="../jinx-loongarch64";;
esac

JINX_DIR="$(realpath "${BUILD_DIR}")"
JINX_SCRIPT="$(realpath ../jinx/jinx)"
STAGE_DIR="${PWD}/initramfs-${LIMINE_ARCH}"
ISO_DIR="$(realpath ./iso/)"

PACKAGES=(
    klib
    helloworld
)

mkdir -p "${STAGE_DIR}"

cd "${JINX_DIR}"
"${JINX_SCRIPT}" build      "${PACKAGES[@]}"
"${JINX_SCRIPT}" install -f "${STAGE_DIR}" "${PACKAGES[@]}"

tar --sort=name                         \
    --mtime='UTC 2026-01-01'            \
    --owner=0 --group=0 --numeric-owner \
    -C "${STAGE_DIR}"                   \
    -cf "${ISO_DIR}/initramfs-${LIMINE_ARCH}.tar" .
