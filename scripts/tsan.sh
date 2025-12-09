#!/usr/bin/env bash
set -euo pipefail
cmake --preset tsan
cmake --build --preset tsan -j
./build/tsan/systems-dsa "$@"
