#!/usr/bin/env bash
set -euo pipefail
cmake --preset release
cmake --build --preset release -j
exec ./build/release/benchmarks/cpp-project-template "$@"
