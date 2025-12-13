#!/usr/bin/env bash
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

preset="${1:?usage: scripts/use-preset-db.sh <libcxx|asan|tsan|release>}"
ln -sf "$REPO_ROOT/build/$preset/compile_commands.json" "$REPO_ROOT/compile_commands.json"
echo "compile_commands.json -> build/$preset/compile_commands.json"
