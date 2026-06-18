#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"
JINX_DIR="$(realpath ../jinx/)"
ISO_DIR="$(realpath ./iso/)"

mkdir -p ${ISO_DIR}/EFI/BOOT/
cp ${JINX_DIR}/host-pkgs/limine/usr/local/share/limine/BOOTX64.EFI         ${ISO_DIR}/EFI/BOOT/
cp ${JINX_DIR}/host-pkgs/limine/usr/local/share/limine/BOOTLOONGARCH64.EFI ${ISO_DIR}/EFI/BOOT/
cp ${JINX_DIR}/host-pkgs/limine/usr/local/share/limine/BOOTRISCV64.EFI     ${ISO_DIR}/EFI/BOOT/
cp ${JINX_DIR}/host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin  ${ISO_DIR}
cp ${JINX_DIR}/host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin  ${ISO_DIR}
cp ${JINX_DIR}/host-pkgs/limine/usr/local/share/limine/limine-bios.sys     ${ISO_DIR}
cp ${JINX_DIR}/host-pkgs/ovmf2-bin/ovmf-code-x86_64.fd .
cp ${JINX_DIR}/host-pkgs/ovmf2-bin/ovmf-vars-x86_64.fd .
cp ${JINX_DIR}/host-pkgs/ovmf2-bin/ovmf-code-loongarch64.fd .
cp ${JINX_DIR}/host-pkgs/ovmf2-bin/ovmf-vars-loongarch64.fd .
cp ${JINX_DIR}/host-pkgs/ovmf2-bin/ovmf-code-riscv64.fd .
cp ${JINX_DIR}/host-pkgs/ovmf2-bin/ovmf-vars-riscv64.fd .

xorriso \
  -as mkisofs \
  -V "EvalynOS"\
  -R \
  -r \
  -J \
  -b limine-bios-cd.bin \
  -no-emul-boot \
  -boot-load-size 4 \
  -boot-info-table \
  -hfsplus \
  -apm-block-size 2048 \
  --efi-boot limine-uefi-cd.bin \
  -efi-boot-part \
  --efi-boot-image \
  --protective-msdos-label \
  ./iso \
  -o evalynOS.iso

cp ./evalynOS.iso ../evalynOS.iso

${JINX_DIR}/host-pkgs/limine/usr/local/bin/limine bios-install evalynOS.iso
