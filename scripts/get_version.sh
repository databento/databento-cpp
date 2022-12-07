#! /usr/bin/env bash

SCRIPTS_DIR="$(cd "$(dirname "$0")" || exit; pwd -P)"
PROJECT_ROOT_DIR="$(dirname "${SCRIPTS_DIR}")"
grep '^project("databento" VERSION' "${PROJECT_ROOT_DIR}/CMakeLists.txt" | cut -d' ' -f3
