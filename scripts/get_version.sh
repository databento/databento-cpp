#! /usr/bin/env bash

SCRIPTS_DIR="$(cd "$(dirname "$0")" || exit; pwd -P)"
PROJECT_ROOT_DIR="$(dirname "${SCRIPTS_DIR}")"
grep --after-context=2 '^project($' "${PROJECT_ROOT_DIR}/CMakeLists.txt" \
  | grep --perl-regexp --only-matching 'VERSION \d+\.\d+\.\d+' \
  | cut --delimiter=' ' --fields=2
