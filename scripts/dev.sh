#!/usr/bin/env bash
set -euo pipefail
cmake --preset libcxx
cmake --build --preset libcxx -j
