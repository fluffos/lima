#!/bin/bash
set -euo pipefail

rm -rf build
mkdir -p build && cd build

# fix me, should use cmake variables
cp -f local_options driver/src/

cmake ../driver -DPACKAGE_UIDS=OFF

make install
