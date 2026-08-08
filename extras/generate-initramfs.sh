#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

JINX_DIR="$(realpath ../jinx/)"
SRC_DIR="$(realpath ./initramfs)"
ISO_DIR="$(realpath ./iso/)"

PACKAGES=(

)

cd ${JINX_DIR}
./jinx build      "${PACKAGES[@]}"
./jinx install -f "$SRC_DIR" "${PACKAGES[@]}"

tar --sort=name \
    --mtime='UTC 2026-01-01' \
    --owner=0 --group=0 --numeric-owner \
    -C "${SRC_DIR}" \
    -cf "${ISO_DIR}/initramfs.tar" .


