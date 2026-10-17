#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname -- "$0")/.."

CONFIG=".chariot.json"
# ensure the config exists and isnt empty/whitespace
[ ! -s "${CONFIG}" ] && echo "{}" > "${CONFIG}"
[ -z "$(grep -l '[^[:space:]]' "$CONFIG")" ] && echo "{}" > "$CONFIG"

config_is_null() {
    local result
    result=$(jq -r --arg key "$1" '.[$key] | if . == null then "NULL" else "!NULL" end' "$CONFIG") || exit $?
    [[ "$result" == "NULL" ]]
}

config_read() {
    local cmd="$1"
    jq -r "${cmd}" "${CONFIG}"
}

config_write() {
    local tmp && tmp=$(mktemp) || exit $?
    jq -c "$@" "${CONFIG}" > "${tmp}"
    mv -f "${tmp}" "${CONFIG}"
}

config_read_or() {
    local key="$1"
    local default="$2"

    if config_is_null "${key}"; then
        config_write --arg key "${key}" --arg default "$default" '.[$key] = $default'
        printf '%s\n' "$default"
    else
        config_read ".${key}"
    fi
}

if [ -v LSP_MODE ]; then
    PACKAGE=$(config_read_or '_package_' "kernel")
    ARCH=$(config_read_or '_arch_' "x86_64")
    [ "$PACKAGE" = "kernel" ] && exec clangd
    exec ./chariot/chariot-linux-x86_64 --local-config-format json lsp --allow-new-profiles --arch "${ARCH}" "${LSP:-clangd}" "${PACKAGE}"
fi

if [ -n "${PKG:-}" ]; then
    config_write --arg pkg "${PKG}" '.pkgs[$pkg].enable_build_cache = true'
    config_write --arg pkg "${PKG}" '._package_ = $pkg'
    config_write --arg arch "${ARCH:-x86_64}" '._arch_ = $arch'
fi

if [ -n "${SRC:-}" ]; then
    config_write --arg name "${SRC%%=*}" --arg path "${SRC#*=}" '.source_overrides[$name] = $path'
fi
