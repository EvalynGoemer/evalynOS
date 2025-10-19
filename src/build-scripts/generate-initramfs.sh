#!/usr/bin/env bash
SRC_DIR="./src/initramfs"
OUT_FILE="./initramfs.tar"
HASH_FILE="./src/generated/initramfs.hash"

current_hash=$(find "$SRC_DIR" -type f -exec md5sum {} + | sort | md5sum | awk '{print $1}')

if [[ -f "$HASH_FILE" ]]; then
    previous_hash=$(cat "$HASH_FILE")
else
    previous_hash=""
fi

if [[ "$current_hash" != "$previous_hash" ]]; then
    tar -C "$SRC_DIR" --blocking-factor=1 -cf "$OUT_FILE" .
    echo "initramfs.tar Generated"
    echo "$current_hash" > "$HASH_FILE"
else
    echo "No changes detected, skipping initramfs generation."
fi
