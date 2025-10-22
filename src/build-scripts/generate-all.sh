#!/usr/bin/env bash

./src/build-scripts/generate-symbols.py
./src/build-scripts/compile-user-apps.sh
./src/build-scripts/generate-initramfs.sh
