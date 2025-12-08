#!/usr/bin/env bash
set -euo pipefail

PRESET="${1:-libcxx}"

cmake --preset "$PRESET"
cmake --build --preset "$PRESET" -j

ctest --test-dir "build/$PRESET" --output-on-failure -j
