#!/usr/bin/env bash
set -euo pipefail

# Always run from repo root, even if invoked elsewhere
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "==> Configuring (asan preset)"
cmake --preset asan
ln -sf "$REPO_ROOT/build/asan/compile_commands.json" "$REPO_ROOT/compile_commands.json"

echo "==> Building"
cmake --build --preset asan -j

SCRATCH_BIN="$REPO_ROOT/build/asan/systems_dsa_scratch"

if [[ ! -x "$SCRATCH_BIN" ]]; then
  echo "ERROR: scratch binary not found: $SCRATCH_BIN"
  exit 1
fi

echo "==> Running scratch (ASAN enabled)"
"$SCRATCH_BIN" "$@"
