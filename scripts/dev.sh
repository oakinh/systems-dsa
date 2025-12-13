#!/usr/bin/env bash
set -euo pipefail

# Always run from repo root
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "==> Configuring (libcxx / debug preset)"
cmake --preset libcxx
ln -sf "$REPO_ROOT/build/libcxx/compile_commands.json" "$REPO_ROOT/compile_commands.json"


echo "==> Building"
cmake --build --preset libcxx -j

SCRATCH_BIN="$REPO_ROOT/build/libcxx/systems_dsa_scratch"

if [[ ! -x "$SCRATCH_BIN" ]]; then
  echo "ERROR: scratch binary not found: $SCRATCH_BIN"
  exit 1
fi

echo "==> Running scratch (debug, no sanitizers)"
"$SCRATCH_BIN" "$@"
