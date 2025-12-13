#!/usr/bin/env bash
set -euo pipefail

# Always run from repo root
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

# Default preset if not provided
PRESET="${1:-libcxx}"

echo "==> Configuring (preset: $PRESET)"
cmake --preset "$PRESET"

# Keep compile_commands.json aligned for tools (harmless for Nova)
if [[ -f "$REPO_ROOT/build/$PRESET/compile_commands.json" ]]; then
  ln -sf "$REPO_ROOT/build/$PRESET/compile_commands.json" "$REPO_ROOT/compile_commands.json"
fi

echo "==> Building"
cmake --build --preset "$PRESET" -j

echo "==> Running tests"
ctest \
  --test-dir "$REPO_ROOT/build/$PRESET" \
  --output-on-failure \
  -j
