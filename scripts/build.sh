#! /usr/bin/env bash
set -euo pipefail
set -x

# Control the parallelism with the `NPROC` env
# If unset, it defaults to the smaller of `nproc` or 8
if [ -z "${NPROC+x}" ]; then
    NPROC="$(nproc)"
    NPROC="$(( $NPROC > 8 ? 8 : $NPROC ))"
fi

cmake -S . -B build \
  -DDATABENTO_ENABLE_UNIT_TESTING=1 \
  -DDATABENTO_ENABLE_EXAMPLES=1 \
  -DDATABENTO_ENABLE_CLANG_TIDY=1 \
  -DDATABENTO_ENABLE_ASAN=1 \
  -DDATABENTO_ENABLE_UBSAN=1
cmake --build build -- -j "$NPROC"
