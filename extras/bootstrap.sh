#!/usr/bin/env bash

set -e

cd "$(dirname -- "$0")"

JINX_DIR="$(realpath ../jinx/)"

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

clone_repo_commit                                      \
    https://codeberg.org/Mintsuki/jinx.git             \
    "$JINX_DIR"                                        \
    6940f35b6031df4aa7d06c94d968e674ad93019e

cd "$JINX_DIR"
./jinx init ..
./jinx host-build limine
./jinx host-build ovmf2-bin
