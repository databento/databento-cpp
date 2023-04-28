#! /usr/bin/env bash
set -e

cmake -S . -B build \
  -DDATABENTO_ENABLE_UNIT_TESTING=1 \
  -DDATABENTO_ENABLE_EXAMPLES=1 \
  -DDATABENTO_ENABLE_ASAN=1 \
  -DDATABENTO_ENABLE_UBSAN=1
cmake --build build -- -j "$(nproc)"
