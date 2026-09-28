#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname -- "$0")/.."

HOST_PACKAGES="${PWD}/extras/host-packages"
QEMU_DIR="${HOST_PACKAGES}/qemu"
QEMU_HASH="$(sha256sum "${PWD}/dist/host/qemu/qemu.lua" | cut -d' ' -f1)"

if [ "$(cat "${QEMU_DIR}/.stamp" 2>/dev/null)" != "${QEMU_HASH}" ]; then
    ./chariot/chariot-linux-x86_64 install --arch x86_64 --allow-new-profiles --tool --force qemu "${QEMU_DIR}"
    echo "${QEMU_HASH}" > "${QEMU_DIR}/.stamp"
fi

SHARED_ENVIRONMENT=(
    DISPLAY WAYLAND_DISPLAY XDG_RUNTIME_DIR PULSE_SERVER DBUS_SESSION_BUS_ADDRESS GDK_BACKEND
)

SHARED_MOUNTS=(
    "/dev/kvm=/dev/kvm:file"
    "/tmp/.X11-unix=/tmp/.X11-unix"
    "${HOME}/.config/gtk-3.0=/home/chariot/.config/gtk-3.0:ro"
    "${XDG_RUNTIME_DIR:-}=${XDG_RUNTIME_DIR:-}"
)

opts=(
    --arch x86_64
    --allow-new-profiles
    --native-pkg "gtk4 vte3 dtc libpulse"
    --cwd /proj
    --stdin
    --env-var "XDG_CACHE_HOME=/tmp"
    --env-var "HOME=/home/chariot"
    --mount   "$PWD=/proj"
    --mount   "$QEMU_DIR/usr/local=/usr/local"
)

for var in "${SHARED_ENVIRONMENT[@]}"; do
    [[ -n ${!var:-} ]] && opts+=(-e "${var}=${!var}")
done

for mount in "${SHARED_MOUNTS[@]}"; do
    [[ -e ${mount%%=*} ]] && opts+=(-m "${mount}")
done

mount_file_env() {
    local path="${!1:-$2}"
    [ -f "${path}" ] || return 0
    path="$(realpath -- "${path}")"
    opts+=(-m "${path}=${path}:file")
}

mount_file_env XAUTHORITY   "${HOME}/.Xauthority"
mount_file_env PULSE_COOKIE "${HOME}/.config/pulse/cookie"

./chariot/chariot-linux-x86_64 exec "${opts[@]}" -- "$(printf '%q ' "$@")" \
    2> >(sed '/^WARNING: Glycin running without sandbox\.$/d' >&2)
