#! /usr/bin/env bash
cd build
ctest --timeout 300 --output-on-failure --tests-regex databentoTests
