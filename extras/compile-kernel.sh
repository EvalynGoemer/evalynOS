#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"

JINX_DIR="$(realpath ../jinx/)"
KERNEL_DIR="$(realpath ../kernel)"
ISO_DIR="$(realpath ./iso/)"

cd ${KERNEL_DIR}
make -j$(nproc) ARCH="${ARCH}"
cp ${KERNEL_DIR}/bin-${ARCH}/kernel.elf ${ISO_DIR}
