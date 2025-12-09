#!/usr/bin/env bash
set -euo pipefail
cmake --preset asan
cmake --build --preset asan -j
./build/asan/systems-dsa "$@"
