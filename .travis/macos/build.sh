#!/bin/bash -ex

set -o pipefail

export MACOSX_DEPLOYMENT_TARGET=10.14
export Qt5_DIR=$(brew --prefix)/opt/qt5
export PATH="/usr/local/opt/ccache/libexec:$PATH"

# TODO: Build using ninja instead of make
mkdir build && cd build
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64h" -DCMAKE_BUILD_TYPE=Release -DENABLE_FFMPEG_AUDIO_DECODER=ON -DENABLE_FFMPEG_VIDEO_DUMPER=ON
make -j4
