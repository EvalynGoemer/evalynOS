#!/usr/bin/env bash

set -euo pipefail

die() {
    printf 'container-run: %s\n' "$@" >&2
    exit 1
}

SCRIPT="$(realpath -- "$0")"
PROJECT="$(realpath -- "$(dirname -- "${SCRIPT}")/..")"
CACHE="${PROJECT}/.jinx-cache"
BASE="${CACHE}/sets/.image"

SHARED_ENVIRONMENT=(
    DISPLAY WAYLAND_DISPLAY XDG_RUNTIME_DIR PULSE_SERVER
    DBUS_SESSION_BUS_ADDRESS GDK_BACKEND
)
CHROOT_ENV=(
    PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    LD_LIBRARY_PATH=/usr/local/lib64:/usr/local/lib:/usr/lib64:/usr/lib
    HOME=/root LANG=C LC_ALL=C DEBIAN_FRONTEND=noninteractive
    LD_PRELOAD=/nochown.so
)

mount_tree() {
    local source="$1" target="$2" mode="${3:-ro}"
    [ -d "${source}" ] || return 0
    mount -m --rbind "${source}" "${target}"
    [ "${mode}" = ro ] && mount -o remount,ro,bind "${target}" || :
}

mount_file_env() {
    local path="${!1:-$2}"
    [ -f "${path}" ] || return 0
    path="$(realpath -- "${path}")"
    mkdir -p "${root}$(dirname -- "${path}")"
    touch "${root}${path}"
    mount -o ro --bind "${path}" "${root}${path}"
    CHROOT_ENV+=("$1=${path}")
}

mount_runtime() {
    local root="$1"

    touch "${root}/etc/resolv.conf"
    mount -o ro --bind /etc/resolv.conf "${root}/etc/resolv.conf"
    for i in dev sys proc; do
        mount -m --rbind "/${i}" "${root}/${i}"
    done

    mount -t tmpfs run "${root}/run"
    mount -t tmpfs tmpfs "${root}/tmp"
    mount -t tmpfs tmpfs "${root}/var/tmp"
}

setup_child() {
    local root="$2" apt_cache="$3"; shift 3
    mkdir -p "${root}/var/cache/apt/archives"
    mount --bind "${apt_cache}" "${root}/var/cache/apt/archives"
    mount_runtime "${root}"
    chroot "${root}" /usr/bin/env -i "${CHROOT_ENV[@]}" /bin/bash -c 'apt-get update && apt-get install -y --no-install-recommends "$@" && apt-get clean' bash "$@"
}

run_child() {
    local root="$2" package_root="$3" project="$4"; shift 4

    mkdir -p "${root}/proj" "${root}/usr/local"
    mount --bind "${project}" "${root}/proj"
    mount -o ro --bind "${package_root}/usr/local" "${root}/usr/local"
    mount_runtime "${root}"

    mount_tree "${HOME:-}/.config/gtk-3.0" "${root}/root/.config/gtk-3.0" ro
    mount_tree /usr/share/themes "${root}/usr/share/themes" ro
    mount_tree /usr/share/icons "${root}/usr/share/icons" ro
    mount_tree /usr/share/fonts "${root}/usr/share/fonts" ro
    [ -n "${DISPLAY:-}" ] && mount_tree /tmp/.X11-unix "${root}/tmp/.X11-unix" ro
    [ -n "${XDG_RUNTIME_DIR:-}" ] && mount_tree "${XDG_RUNTIME_DIR}" "${root}${XDG_RUNTIME_DIR}" rw

    mount_file_env XAUTHORITY   ${HOME}/.Xauthority
    mount_file_env PULSE_COOKIE ${HOME}/.config/pulse/cookie

    local shared_variable
    for shared_variable in "${SHARED_ENVIRONMENT[@]}"; do
        [[ -n ${!shared_variable:-} ]] && CHROOT_ENV+=("${shared_variable}=${!shared_variable}")
    done

    chroot "${root}" /usr/bin/env -i "${CHROOT_ENV[@]}" /bin/bash -c 'cd /proj; exec "$@" 2> >(sed "/^WARNING: Glycin running without sandbox\.$/d" >&2)' bash "$@"
}

ensure_root() {
    local root="$1" stamp="$2" apt_cache="$3"; shift 3
    [ -f "${root}/.container-run-stamp" ] && current_stamp="$(<"${root}/.container-run-stamp")" || current_stamp=""
    [ "${current_stamp}" = "${stamp}" ] && return

    if command -v rsync >/dev/null; then
        rsync -a --del "${BASE}/" "${root}/"
    else
        rm -rf "${root}"
        cp -a "${BASE}/." "${root}/"
    fi

    unshare -Urmpf -- "${SCRIPT}" --setup-child "${root}" "${apt_cache}" "$@"
    printf '%s\n' "${stamp}" > "${root}/.container-run-stamp"
}

run() {
    local package="$1"; shift 1

    local name="${package#host:}"
    local recipe="${PROJECT}/host-recipes/${name}/recipe"
    local package_root="${PROJECT}/jinx/host-pkgs/${name}"
    local root="${CACHE}/containers/host"

    [ -f "${recipe}" ] || die "Recipe not found: ${recipe}"
    [ -d "${BASE}" ] || die "Jinx base image not found: ${BASE}"
    [ -d "${package_root}/usr/local" ] || die "Package not extracted: ${package_root}"

    local imagedeps="$(sed -n 's/^imagedeps="\(.*\)"$/\1/p' "${PROJECT}"/host-recipes/*/recipe | sort -u | tr '\n' ' ')"
    local stamp="$(printf '%s\n' "${imagedeps}" "$(cat "${PROJECT}/Jinxfile")" | b2sum | cut -d' ' -f1)"
    local -a dependencies; read -r -a dependencies <<< "${imagedeps:-}"

    mkdir -p "$(dirname -- "${root}")" "${CACHE}/apt-cache"
    ensure_root "${root}" "${stamp}" "${CACHE}/apt-cache" "${dependencies[@]}"

    exec unshare -Urmpf -- "${SCRIPT}" --run-child "${root}" "${package_root}" "${PROJECT}" "$@"
}

case "${1-}" in
    --setup-child) setup_child "$@" ;;
    --run-child) run_child "$@" ;;
    *) run "$@" ;;
esac
