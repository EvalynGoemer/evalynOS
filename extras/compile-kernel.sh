#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

ARCH="${ARCH:-x86_64}"
KASLR="${KASLR:-true}"
LA64_PAGESIZE="${LA64_PAGESIZE:-16KB}"

PACKAGES_DIR="$(realpath ./host-packages)"
KERNEL_DIR="$(realpath ../kernel)"
ISO_DIR="$(realpath ./iso/)"
ARGS_FILE="$(realpath .build_args.stamp)"

CC="${PACKAGES_DIR}/llvm/usr/local/bin/clang"
LD="${PACKAGES_DIR}/llvm/usr/local/bin/ld.lld"
NASM="${PACKAGES_DIR}/nasm/usr/local/bin/nasm"

cd ${KERNEL_DIR}

CURRENT_ARGS="ARCH=${ARCH} KASLR=${KASLR} LA64_PAGESIZE=${LA64_PAGESIZE} CC=${CC} LD=${LD} NASM=${NASM}"
if [ -f "${ARGS_FILE}" ]; then
    LAST_ARGS="$(cat "${ARGS_FILE}")"
    if [ "${LAST_ARGS}" != "${CURRENT_ARGS}" ]; then
        make clean ARCH="${ARCH}"
    fi
fi
echo "${CURRENT_ARGS}" > "${ARGS_FILE}"

make -j$(nproc) ARCH="${ARCH}" KASLR="${KASLR}" LA64_PAGESIZE="${LA64_PAGESIZE}" CC="${CC}" LD="${LD}" NASM="${NASM}"
cp ${KERNEL_DIR}/bin-${ARCH}/kernel.elf ${ISO_DIR}/kernel-${ARCH//_/-}.elf
