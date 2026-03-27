#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

PRESET="tsan"
BUILD_DIR="$REPO_ROOT/build/$PRESET"

echo "==> Configuring ($PRESET preset)"
cmake --preset "$PRESET"

if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
  ln -sf "$BUILD_DIR/compile_commands.json" "$REPO_ROOT/compile_commands.json"
fi

echo "==> Building"
cmake --build --preset "$PRESET" -j

echo "==> Running tests (TSAN enabled)"
TSAN_OPTIONS="halt_on_error=1" \
ctest \
  --test-dir "$BUILD_DIR" \
  --output-on-failure \
  -j