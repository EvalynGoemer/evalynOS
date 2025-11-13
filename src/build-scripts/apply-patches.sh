#!/bin/sh

apply_patch() {
    printf "Applying %s to %s\n" "$2" "$1"
    git -C $1 apply ../../$2
}

if [ ! -f deps/flanterm/src/flanterm_backends/fb.c.tmp ]; then
    cp deps/flanterm/src/flanterm_backends/fb.c deps/flanterm/src/flanterm_backends/fb.c.tmp
fi
apply_patch deps/flanterm src/patches/flanterm-fb-balloc-size-increase.patch

printf "Patches successfully applied.\n"
