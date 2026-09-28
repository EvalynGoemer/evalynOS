#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"
depsdir="$(realpath deps)"
mkdir -p "${depsdir}"

clone_repo_commit() {
    if test -d "$2/.git"; then
        git -C "$2" reset --hard
        git -C "$2" clean -fd
        if ! git -C "$2" -c advice.detachedHead=false checkout $3; then
            rm -rf "$2"
        fi
    else
        if test -d "$2"; then
            echo "error: '$2' is not a Git repository" 1>&2
            exit 1
        fi
    fi
    if ! test -d "$2"; then
        git clone $1 "$2"
        if ! git -C "$2" -c advice.detachedHead=false checkout $3; then
            rm -rf "$2"
            exit 1
        fi
    fi
}

clone_repo_commit                                             \
    https://github.com/Mintsuki/Flanterm.git                  \
    "${depsdir}/flanterm"                                     \
    e231edd0e508022521c8233392aabc8ce5c9b242

clone_repo_commit                                             \
    https://github.com/Limine-Bootloader/limine-protocol.git  \
    "${depsdir}/limine-protocol"                              \
    630686a3dd3ce40f9e510a7dd9fea6b4c60d952e

clone_repo_commit                                             \
    https://github.com/osdev0/freestanding-c-hdrs.git         \
    "${depsdir}/freestnd-c-hdrs"                              \
    38fed4e1e3365733ddbfa03b0a28936243ad31e9

clone_repo_commit                                             \
    https://github.com/osdev0/cc-runtime.git                  \
    "${depsdir}/cc-runtime"                                   \
    dae79833b57a01b9fd3e359ee31def69f5ae899b

dtc_commit="8f48565e5cfedc74d3f7512f1e0188e9d85dc1de"
dtc_stamp="${depsdir}/libfdt/.got_dtc-${dtc_commit}.stamp"

if [ ! -f "${dtc_stamp}" ]; then
    clone_repo_commit                                         \
        https://github.com/dgibson/dtc.git                    \
        "${depsdir}/dtc"                                      \
        "$dtc_commit"

    cp "${depsdir}/dtc/libfdt/fdt.h" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/libfdt.h" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/libfdt_internal.h" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/fdt.c" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/fdt_ro.c" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/fdt_addresses.c" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/fdt_strerror.c" "${depsdir}/libfdt"
    cp "${depsdir}/dtc/libfdt/fdt_check.c" "${depsdir}/libfdt"

    rm -rf "${depsdir}/dtc"

    touch "${dtc_stamp}"
fi
