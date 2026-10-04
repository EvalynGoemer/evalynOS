#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname -- "$0")"

CHARIOT_URL="https://github.com/chariot-build/chariot/releases/download/2026-10-03/chariot-linux-x86_64"
CHARIOT_HASH="bfcf206cebadc7f464463e0d9b3c60c79da69600ad32981c40e3656ea33e9fdf"
CHARIOT_DIR="$(realpath ../chariot/)"
CHARIOT="${CHARIOT_DIR}/chariot-linux-x86_64"

mkdir -p "$CHARIOT_DIR"
wget "${CHARIOT_URL}" -O "${CHARIOT}"

if [ "${CHARIOT_HASH}" != $(sha256sum "${CHARIOT}" | cut -d' ' -f1) ]; then
    echo "ERROR: chariot binary did not have the expected hash of ${CHARIOT_HASH}" && exit 1
fi

chmod +x "${CHARIOT}"

PACKAGES_DIR="$(realpath ./host-packages)"
PROJECT_DIR="$(realpath ../)"
KERNEL_DIR="$(realpath ../kernel/)"

cd "${PROJECT_DIR}"

${CHARIOT} support setup-lsp --support-dir .chariot-lsp-support

host_pkgs=(limine llvm nasm ovmf2 xorriso)

echo "building host packages"

${CHARIOT} build --arch x86_64 --allow-new-profiles --tool "${host_pkgs[@]}"

for pkg in "${host_pkgs[@]}"; do
    ${CHARIOT} install --arch x86_64 --allow-new-profiles --tool --force "${pkg}" "${PACKAGES_DIR}/${pkg}"
done


cd "$KERNEL_DIR"
echo "getting kernel deps"
./get-deps.sh

cd "$PROJECT_DIR"
echo "generating initramfs"
make initramfs
echo "generating iso"
make mkiso

echo "finished bootstrap"
