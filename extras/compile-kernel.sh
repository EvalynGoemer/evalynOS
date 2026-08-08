#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"
KASLR="${KASLR:-true}"

KERNEL_DIR="$(realpath ../kernel)"
ISO_DIR="$(realpath ./iso/)"
ARGS_FILE="$(realpath .build_args.stamp)"

cd ${KERNEL_DIR}

CURRENT_ARGS="ARCH=${ARCH} KASLR=${KASLR}"
if [ -f "${ARGS_FILE}" ]; then
    LAST_ARGS="$(cat "${ARGS_FILE}")"
    if [ "${LAST_ARGS}" != "${CURRENT_ARGS}" ]; then
        make clean ARCH="${ARCH}"
    fi
fi
echo "${CURRENT_ARGS}" > "${ARGS_FILE}"

make -j$(nproc) ARCH="${ARCH}" KASLR="${KASLR}"
cp ${KERNEL_DIR}/bin-${ARCH}/kernel.elf ${ISO_DIR}/kernel-${ARCH//_/-}.elf
