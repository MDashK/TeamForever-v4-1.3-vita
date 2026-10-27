#!/bin/bash
# Builds Sonic1Forever.vpk and Sonic2Absolute.vpk (run inside WSL/Linux with VitaSDK installed).
set -e
export VITASDK=${VITASDK:-/usr/local/vitasdk}
export PATH=$VITASDK/bin:$PATH
cd "$(dirname "$0")"

cmake -S . -B build-vita -DCMAKE_BUILD_TYPE=Release > /dev/null
# vita.cmake doesn't track the elf -> velf/self/vpk dependency, so always regenerate the packages
rm -f build-vita/*.velf build-vita/*.self build-vita/*.vpk build-vita/*.vpk_param.sfo
cmake --build build-vita -j"$(nproc)"
ls -la build-vita/*.vpk
