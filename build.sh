#!/bin/bash
set -euo pipefail

rm -rf build
mkdir -p build && cd build

cmake ../driver

make install
