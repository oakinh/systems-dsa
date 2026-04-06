#!/usr/bin/env bash
set -euo pipefail
cmake --preset release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
