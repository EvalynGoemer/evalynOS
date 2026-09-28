#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

case "${ARCH}" in
    x86_64)      LIMINE_ARCH="x86-64";      ;;
    riscv64)     LIMINE_ARCH="riscv64";     ;;
    loongarch64) LIMINE_ARCH="loongarch64"; ;;
esac

CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

STAGE_DIR="$(realpath ./initramfs-${LIMINE_ARCH})"
PROJECT_DIR="$(realpath ../)"
ISO_DIR="$(realpath ./iso/)"

PACKAGES=(
    klib
    helloworld
)

cd ${PROJECT_DIR}

for pkg in "${PACKAGES[@]}"; do
    ${CHARIOT} install --arch ${ARCH} --allow-new-profiles --force "${pkg}" "${STAGE_DIR}"
done

tar --sort=name                         \
    --mtime='UTC 2026-01-01'            \
    --owner=0 --group=0 --numeric-owner \
    -C "${STAGE_DIR}"                   \
    -cf "${ISO_DIR}/initramfs-${LIMINE_ARCH}.tar" .
