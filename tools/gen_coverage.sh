#!/usr/bin/env bash
set -euo pipefail

WORKSPACE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="${WORKSPACE}/coverage"
mkdir -p "$OUTPUT_DIR"

bazel coverage \
    --config=gcc \
    --combined_report=lcov \
    --instrumentation_filter=//src \
    //test/...

LCOV_FILE="${OUTPUT_DIR}/coverage.lcov"
cp "$(bazel info output_path)/_coverage/_coverage_report.dat" "$LCOV_FILE"

HTML_DIR="${OUTPUT_DIR}/html"
genhtml --branch-coverage --ignore-errors unsupported,inconsistent --output-directory "$HTML_DIR" "$LCOV_FILE"

echo "LCOV report:  ${LCOV_FILE}"
echo "HTML report:  ${HTML_DIR}/index.html"
