#!/usr/bin/env bash
set -euo pipefail

echo "==> Installing base build tools"
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  gdb \
  lld \
  git \
  ripgrep

echo "==> Installing LLVM toolchain (clang 19)"
sudo apt install -y \
  clang-19 \
  clang++-19 \
  clang-tidy-19 \
  clang-format-19 \
  clangd-19 \
  lldb-19

echo "==> Verifying toolchain"
clang++-19 --version
clang-tidy-19 --version
cmake --version
ninja --version

echo "==> Done"
