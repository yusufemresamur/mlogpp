#!/usr/bin/env bash
set -e

WS="$(bazel info workspace)"
find "${WS}/src/" "${WS}/test/" -name "*.cpp" | xargs bazel run @llvm_toolchain//:clang-tidy -- -p "${WS}"
