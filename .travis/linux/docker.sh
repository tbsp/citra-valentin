#!/bin/bash -ex

cd /citra

mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_FFMPEG_VIDEO_DUMPER=ON -DCITRA_ENABLE_DISCORD_RP=ON
ninja

ctest -VV -C Release
