#!/usr/bin/env bash
set -euo pipefail

# Always run from repo root
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

PRESET="bench"
BUILD_DIR="$REPO_ROOT/build/$PRESET"
BENCH_BIN="$BUILD_DIR/benchmarks/systems_dsa_bench"

echo "==> Configuring ($PRESET preset)"
cmake --preset "$PRESET"

# Keep tooling (clangd/CLion) pointed at the active build
if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
  ln -sf "$BUILD_DIR/compile_commands.json" "$REPO_ROOT/compile_commands.json"
fi

echo "==> Building"
cmake --build --preset "$PRESET" -j

if [[ ! -x "$BENCH_BIN" ]]; then
  echo "ERROR: benchmark binary not found or not executable: $BENCH_BIN"
  echo "Hint: check that SYSTEMS_DSA_BUILD_BENCHMARKS=ON in the '$PRESET' preset,"
  echo "      and that benchmarks/CMakeLists.txt adds the systems_dsa_bench target."
  exit 1
fi

echo "==> Running benchmarks"
exec "$BENCH_BIN" "$@"
