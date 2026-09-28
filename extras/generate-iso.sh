#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

PACKAGES_DIR="$(realpath ./host-packages)"
ISO_DIR="$(realpath ./iso/)"

mkdir -p ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/BOOTX64.EFI         ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/BOOTIA32.EFI        ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/BOOTLOONGARCH64.EFI ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/BOOTRISCV64.EFI     ${ISO_DIR}/EFI/BOOT/
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/limine-bios-cd.bin  ${ISO_DIR}
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/limine-uefi-cd.bin  ${ISO_DIR}
cp ${PACKAGES_DIR}/limine/usr/local/share/limine/limine-bios.sys     ${ISO_DIR}
cp ${PACKAGES_DIR}/ovmf2/ovmf-code-x86_64.fd .
cp ${PACKAGES_DIR}/ovmf2/ovmf-vars-x86_64.fd .
cp ${PACKAGES_DIR}/ovmf2/ovmf-code-loongarch64.fd .
cp ${PACKAGES_DIR}/ovmf2/ovmf-vars-loongarch64.fd .
cp ${PACKAGES_DIR}/ovmf2/ovmf-code-riscv64.fd .
cp ${PACKAGES_DIR}/ovmf2/ovmf-vars-riscv64.fd .

"${PACKAGES_DIR}/xorriso/usr/local/bin/xorriso" \
    -as mkisofs -V "EvalynOS" -R -r -J    \
    --modification-date=2026010100000000  \
    --set_all_file_dates 2026010100000000 \
    -hfsplus -apm-block-size 2048         \
    -b limine-bios-cd.bin                 \
    -no-emul-boot                         \
    -boot-load-size 4                     \
    -boot-info-table                      \
    --efi-boot limine-uefi-cd.bin         \
    -efi-boot-part                        \
    --efi-boot-image                      \
    --protective-msdos-label              \
    -o evalynOS.iso                       \
    ${ISO_DIR}

${PACKAGES_DIR}/limine/usr/local/bin/limine bios-install evalynOS.iso

cp ./evalynOS.iso ../evalynOS.iso
