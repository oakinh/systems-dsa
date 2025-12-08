#!/usr/bin/env bash
set -euo pipefail
PRESET="${1:-libcxx}"
cmake --preset "$PRESET"
cmake --build --preset "$PRESET" -j
ln -sf "build/$PRESET/compile_commands.json" ./compile_commands.json
