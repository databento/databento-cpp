#! /usr/bin/env bash
cd build
ctest --timeout 300 --output-on-failure --exclude-regex 'cmake_fetch_content.*'
