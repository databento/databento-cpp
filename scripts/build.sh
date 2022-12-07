#! /usr/bin/env bash
set -e
cmake -S . -B build -DDATABENTO_ENABLE_EXAMPLES=1
cmake --build build -- -j "$(nproc)"
